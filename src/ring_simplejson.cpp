// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifdef _WIN32
#define NOMINMAX
#endif

#include "glaze/base64/base64.hpp"
#include "glaze/cbor.hpp"
#include "glaze/csv.hpp"
#include "glaze/glaze.hpp"
#include "glaze/json/jmespath.hpp"
#include "glaze/json/ndjson.hpp"
#include "glaze/json/patch.hpp"
#include "glaze/toml.hpp"

extern "C"
{
#include "ring.h"
	RING_API void ringlib_init(RingState *pRingState);
}

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

static constexpr const char *GLAZE_VERSION_STRING = "7.0.1";

#define IS_JSON_EMPTY_OBJECT(pList)                                                                                    \
	(ring_list_getsize(pList) == 1 && ring_list_isstring(pList, 1) &&                                                  \
	 strcmp(ring_list_getstring(pList, 1), "__JSON_EMPTY_OBJECT__") == 0)

static glz::generic ring_item_to_json_value(VM *pVM, Item *pItem, List *pVisited);
static glz::generic ring_list_to_json_value(VM *pVM, List *pList, List *pVisited);

static void json_value_to_ring_item(void *pState, const glz::generic &value, List *pList)
{
	if (auto *obj = value.get_if<glz::generic::object_t>())
	{
		List *pObjectList = ring_list_newlist_gc(pState, pList);
		for (const auto &[key, val] : *obj)
		{
			List *pItemList = ring_list_newlist_gc(pState, pObjectList);
			ring_list_addstring_gc(pState, pItemList, key.c_str());
			json_value_to_ring_item(pState, val, pItemList);
		}
	}
	else if (auto *arr = value.get_if<glz::generic::array_t>())
	{
		List *pArrayList = ring_list_newlist_gc(pState, pList);
		for (const auto &val : *arr)
		{
			json_value_to_ring_item(pState, val, pArrayList);
		}
	}
	else if (auto *str = value.get_if<std::string>())
	{
		ring_list_addstring_gc(pState, pList, str->c_str());
	}
	else if (auto *num = value.get_if<double>())
	{
		ring_list_adddouble_gc(pState, pList, *num);
	}
	else if (auto *b = value.get_if<bool>())
	{
		ring_list_adddouble_gc(pState, pList, *b ? 1.0 : 0.0);
	}
	else
	{
		ring_list_addstring_gc(pState, pList, RING_CSTR_EMPTY);
	}
}

RING_FUNC(ring_json_decode)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);

	if (ec)
	{
		std::string errorMsg = "JSON error: " + std::string(glz::format_error(ec, jsonView));
		RING_API_ERROR(errorMsg.c_str());
		return;
	}

	List *pTempList = RING_API_NEWLIST;
	json_value_to_ring_item(RING_API_STATE, jsonRoot, pTempList);

	if (ring_list_getsize(pTempList) == 1 && ring_list_islist(pTempList, 1))
	{
		List *pResultList = ring_list_getlist(pTempList, 1);
		List *pFinalList = RING_API_NEWLIST;
		ring_list_swaptwolists(pFinalList, pResultList);
		RING_API_RETLISTBYREF(pFinalList);
	}
	else
	{
		RING_API_RETLISTBYREF(pTempList);
	}
}

static int is_ring_list_a_json_object(List *pList)
{
	if (IS_JSON_EMPTY_OBJECT(pList))
	{
		return 1;
	}

	if (ring_list_getsize(pList) == 0)
	{
		return 0;
	}

	for (int x = 1; x <= ring_list_getsize(pList); x++)
	{
		if (!ring_list_islist(pList, x))
		{
			return 0;
		}
		List *pSubList = ring_list_getlist(pList, x);

		if (!(ring_list_getsize(pSubList) == 2 && ring_list_isstring(pSubList, 1)))
		{
			return 0;
		}
	}
	return 1;
}

static glz::generic ring_list_to_json_value(VM *pVM, List *pList, List *pVisited)
{
	if (ring_list_findpointer(pVisited, pList))
	{
		return glz::generic{nullptr};
	}
	ring_list_addpointer_gc(pVM->pRingState, pVisited, pList);

	glz::generic result;

	if (IS_JSON_EMPTY_OBJECT(pList))
	{
		result.data = glz::generic::object_t{};
	}
	else if (is_ring_list_a_json_object(pList))
	{
		glz::generic::object_t obj;
		for (int x = 1; x <= ring_list_getsize(pList); x++)
		{
			List *pSubList = ring_list_getlist(pList, x);
			const char *cKey = ring_list_getstring(pSubList, 1);
			Item *pValueItem = ring_list_getitem(pSubList, 2);
			obj[cKey] = ring_item_to_json_value(pVM, pValueItem, pVisited);
		}
		result.data = std::move(obj);
	}
	else
	{
		glz::generic::array_t arr;
		arr.reserve(ring_list_getsize(pList));
		for (int x = 1; x <= ring_list_getsize(pList); x++)
		{
			Item *pItem = ring_list_getitem(pList, x);
			arr.push_back(ring_item_to_json_value(pVM, pItem, pVisited));
		}
		result.data = std::move(arr);
	}

	ring_list_deletelastitem_gc(pVM->pRingState, pVisited);
	return result;
}

static glz::generic ring_item_to_json_value(VM *pVM, Item *pItem, List *pVisited)
{
	glz::generic result;

	switch (ring_item_gettype(pItem))
	{
	case ITEMTYPE_STRING: {
		String *pString = ring_item_getstring(pItem);
		if (ring_string_size(pString) == 0)
		{
			result.data = nullptr;
		}
		else
		{
			result.data = std::string(ring_string_get(pString));
		}
		break;
	}
	case ITEMTYPE_NUMBER: {
		double nNum = ring_item_getnumber(pItem);
		result.data = nNum;
		break;
	}
	case ITEMTYPE_LIST:
		return ring_list_to_json_value(pVM, ring_item_getlist(pItem), pVisited);
	default:
		result.data = nullptr;
		break;
	}

	return result;
}

RING_FUNC(ring_json_encode)
{
	if (RING_API_PARACOUNT < 1 || RING_API_PARACOUNT > 2)
	{
		RING_API_ERROR(RING_API_BADPARACOUNT);
		return;
	}

	if (!RING_API_ISLIST(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	if (RING_API_ISOBJECT(1))
	{
		RING_API_ERROR(
			"Cannot encode a Ring object directly to JSON. Convert the object to a list of attributes first.");
		return;
	}

	bool prettyPrint = false;
	if (RING_API_PARACOUNT == 2)
	{
		if (RING_API_ISNUMBER(2) && RING_API_GETNUMBER(2) == 1.0)
		{
			prettyPrint = true;
		}
	}

	List *pList = RING_API_GETLIST(1);
	VM *pVM = static_cast<VM *>(pPointer);

	List *pVisited = ring_list_new_gc(pVM->pRingState, 0);

	glz::generic jsonRoot = ring_list_to_json_value(pVM, pList, pVisited);
	ring_list_delete_gc(pVM->pRingState, pVisited);

	std::string jsonString;

	if (prettyPrint)
	{
		auto ec = glz::write<glz::opts{.prettify = true}>(jsonRoot, jsonString);
		if (ec)
		{
			RING_API_ERROR("Failed to generate JSON string from list.");
			return;
		}
	}
	else
	{
		auto ec = glz::write_json(jsonRoot, jsonString);
		if (ec)
		{
			RING_API_ERROR("Failed to generate JSON string from list.");
			return;
		}
	}

	RING_API_RETSTRING(jsonString.c_str());
}

RING_FUNC(ring_json_version)
{
	RING_API_RETSTRING(GLAZE_VERSION_STRING);
}

RING_FUNC(ring_json_valid)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic temp;
	auto ec = glz::read_json(temp, jsonView);

	RING_API_RETNUMBER(ec ? 0.0 : 1.0);
}

RING_FUNC(ring_json_minify)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string input(cJsonString);
	std::string result = glz::minify_json(input);

	if (result.empty() && !input.empty())
	{
		RING_API_ERROR("Failed to minify JSON.");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

RING_FUNC(ring_json_prettify)
{
	if (RING_API_PARACOUNT < 1 || RING_API_PARACOUNT > 2)
	{
		RING_API_ERROR(RING_API_BADPARACOUNT);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string input(cJsonString);
	std::string result = glz::prettify_json(input);

	if (result.empty() && !input.empty())
	{
		RING_API_ERROR("Failed to prettify JSON.");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

RING_FUNC(ring_json_pointer)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	const char *cPointer = RING_API_GETSTRING(2);

	std::string_view jsonView(cJsonString);

	glz::generic result;
	auto ec = glz::read_json(result, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	auto *current = &result;
	std::string pointer(cPointer);

	if (!pointer.empty() && pointer[0] == '/')
	{
		pointer = pointer.substr(1);
	}

	std::vector<std::string> tokens;
	size_t pos = 0;
	while ((pos = pointer.find('/')) != std::string::npos)
	{
		tokens.push_back(pointer.substr(0, pos));
		pointer.erase(0, pos + 1);
	}
	if (!pointer.empty())
	{
		tokens.push_back(pointer);
	}

	for (const auto &token : tokens)
	{
		if (auto *obj = current->get_if<glz::generic::object_t>())
		{
			auto it = obj->find(token);
			if (it == obj->end())
			{
				RING_API_ERROR("JSON pointer path not found.");
				return;
			}
			current = &(it->second);
		}
		else if (auto *arr = current->get_if<glz::generic::array_t>())
		{
			size_t index = std::stoull(token);
			if (index >= arr->size())
			{
				RING_API_ERROR("JSON pointer index out of range.");
				return;
			}
			current = &((*arr)[index]);
		}
		else
		{
			RING_API_ERROR("JSON pointer path not found.");
			return;
		}
	}

	List *pTempList = RING_API_NEWLIST;
	json_value_to_ring_item(RING_API_STATE, *current, pTempList);

	if (ring_list_getsize(pTempList) == 1)
	{
		if (ring_list_islist(pTempList, 1))
		{
			List *pResultList = ring_list_getlist(pTempList, 1);
			List *pFinalList = RING_API_NEWLIST;
			ring_list_swaptwolists(pFinalList, pResultList);
			RING_API_RETLISTBYREF(pFinalList);
		}
		else if (ring_list_isstring(pTempList, 1))
		{
			RING_API_RETSTRING(ring_list_getstring(pTempList, 1));
		}
		else if (ring_list_isnumber(pTempList, 1))
		{
			RING_API_RETNUMBER(ring_list_getdouble(pTempList, 1));
		}
		else
		{
			RING_API_RETLISTBYREF(pTempList);
		}
	}
	else
	{
		RING_API_RETLISTBYREF(pTempList);
	}
}

RING_FUNC(ring_json_type)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_RETSTRING("invalid");
		return;
	}

	if (jsonRoot.get_if<glz::generic::object_t>())
	{
		RING_API_RETSTRING("object");
	}
	else if (jsonRoot.get_if<glz::generic::array_t>())
	{
		RING_API_RETSTRING("array");
	}
	else if (jsonRoot.get_if<std::string>())
	{
		RING_API_RETSTRING("string");
	}
	else if (jsonRoot.get_if<double>())
	{
		RING_API_RETSTRING("number");
	}
	else if (jsonRoot.get_if<bool>())
	{
		RING_API_RETSTRING("boolean");
	}
	else
	{
		RING_API_RETSTRING("null");
	}
}

RING_FUNC(ring_json_keys)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	auto *obj = jsonRoot.get_if<glz::generic::object_t>();
	if (!obj)
	{
		RING_API_ERROR("JSON value is not an object.");
		return;
	}

	List *pList = RING_API_NEWLIST;
	for (const auto &[key, val] : *obj)
	{
		ring_list_addstring_gc(RING_API_STATE, pList, key.c_str());
	}

	RING_API_RETLISTBYREF(pList);
}

RING_FUNC(ring_json_has)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	const char *cKey = RING_API_GETSTRING(2);

	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_RETNUMBER(0.0);
		return;
	}

	auto *obj = jsonRoot.get_if<glz::generic::object_t>();
	if (!obj)
	{
		RING_API_RETNUMBER(0.0);
		return;
	}

	RING_API_RETNUMBER(obj->find(cKey) != obj->end() ? 1.0 : 0.0);
}

RING_FUNC(ring_json_merge)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJson1 = RING_API_GETSTRING(1);
	const char *cJson2 = RING_API_GETSTRING(2);

	glz::generic json1, json2;

	auto ec1 = glz::read_json(json1, std::string_view(cJson1));
	if (ec1)
	{
		RING_API_ERROR("Invalid JSON in first argument.");
		return;
	}

	auto ec2 = glz::read_json(json2, std::string_view(cJson2));
	if (ec2)
	{
		RING_API_ERROR("Invalid JSON in second argument.");
		return;
	}

	auto *obj1 = json1.get_if<glz::generic::object_t>();
	auto *obj2 = json2.get_if<glz::generic::object_t>();

	if (!obj1 || !obj2)
	{
		RING_API_ERROR("Both arguments must be JSON objects.");
		return;
	}

	for (const auto &[key, val] : *obj2)
	{
		(*obj1)[key] = val;
	}

	std::string result;
	auto ec = glz::write_json(json1, result);
	if (ec)
	{
		RING_API_ERROR("Failed to serialize merged JSON.");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

RING_FUNC(ring_json_read_file)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cFilePath = RING_API_GETSTRING(1);

	std::ifstream file(cFilePath);
	if (!file.is_open())
	{
		RING_API_ERROR("Cannot open file for reading.");
		return;
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, content);
	if (ec)
	{
		std::string errorMsg = "JSON error: " + std::string(glz::format_error(ec, content));
		RING_API_ERROR(errorMsg.c_str());
		return;
	}

	List *pTempList = RING_API_NEWLIST;
	json_value_to_ring_item(RING_API_STATE, jsonRoot, pTempList);

	if (ring_list_getsize(pTempList) == 1 && ring_list_islist(pTempList, 1))
	{
		List *pResultList = ring_list_getlist(pTempList, 1);
		List *pFinalList = RING_API_NEWLIST;
		ring_list_swaptwolists(pFinalList, pResultList);
		RING_API_RETLISTBYREF(pFinalList);
	}
	else
	{
		RING_API_RETLISTBYREF(pTempList);
	}
}

RING_FUNC(ring_json_write_file)
{
	if (RING_API_PARACOUNT < 2 || RING_API_PARACOUNT > 3)
	{
		RING_API_ERROR(RING_API_BADPARACOUNT);
		return;
	}

	if (!RING_API_ISLIST(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	if (RING_API_ISOBJECT(1))
	{
		RING_API_ERROR("Cannot encode a Ring object directly to JSON.");
		return;
	}

	bool prettyPrint = false;
	if (RING_API_PARACOUNT == 3 && RING_API_ISNUMBER(3))
	{
		prettyPrint = (RING_API_GETNUMBER(3) == 1.0);
	}

	List *pList = RING_API_GETLIST(1);
	const char *cFilePath = RING_API_GETSTRING(2);
	VM *pVM = static_cast<VM *>(pPointer);

	List *pVisited = ring_list_new_gc(pVM->pRingState, 0);
	glz::generic jsonRoot = ring_list_to_json_value(pVM, pList, pVisited);
	ring_list_delete_gc(pVM->pRingState, pVisited);

	std::string jsonString;
	if (prettyPrint)
	{
		auto ec = glz::write<glz::opts{.prettify = true}>(jsonRoot, jsonString);
		if (ec)
		{
			RING_API_ERROR("Failed to generate JSON string.");
			return;
		}
	}
	else
	{
		auto ec = glz::write_json(jsonRoot, jsonString);
		if (ec)
		{
			RING_API_ERROR("Failed to generate JSON string.");
			return;
		}
	}

	std::ofstream file(cFilePath);
	if (!file.is_open())
	{
		RING_API_ERROR("Cannot open file for writing.");
		return;
	}

	file << jsonString;
	file.close();

	RING_API_RETNUMBER(1.0);
}

RING_FUNC(ring_json_to_beve)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	std::string beveData;
	auto ec2 = glz::write_beve(jsonRoot, beveData);
	if (ec2)
	{
		RING_API_ERROR("Failed to convert to BEVE.");
		return;
	}

	RING_API_RETSTRING2(beveData.c_str(), beveData.size());
}

RING_FUNC(ring_beve_to_json)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cBeveData = RING_API_GETSTRING(1);
	int nSize = RING_API_GETSTRINGSIZE(1);

	glz::generic jsonRoot;
	auto ec = glz::read_beve(jsonRoot, std::string_view(cBeveData, nSize));
	if (ec)
	{
		RING_API_ERROR("Invalid BEVE input.");
		return;
	}

	std::string jsonString;
	auto ec2 = glz::write_json(jsonRoot, jsonString);
	if (ec2)
	{
		RING_API_ERROR("Failed to convert to JSON.");
		return;
	}

	RING_API_RETSTRING(jsonString.c_str());
}

// ============================================================================
// JSON Patch (RFC 6902)
// ============================================================================

RING_FUNC(ring_json_patch)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cDocJson = RING_API_GETSTRING(1);
	const char *cPatchJson = RING_API_GETSTRING(2);

	auto result = glz::patch_json(std::string_view(cDocJson), std::string_view(cPatchJson));
	if (!result)
	{
		RING_API_ERROR("Failed to apply JSON patch.");
		return;
	}

	RING_API_RETSTRING(result->c_str());
}

RING_FUNC(ring_json_diff)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cSourceJson = RING_API_GETSTRING(1);
	const char *cTargetJson = RING_API_GETSTRING(2);

	auto patch = glz::diff(std::string_view(cSourceJson), std::string_view(cTargetJson));
	if (!patch)
	{
		RING_API_ERROR("Failed to generate JSON diff.");
		return;
	}

	std::string patchJson;
	auto ec = glz::write_json(*patch, patchJson);
	if (ec)
	{
		RING_API_ERROR("Failed to serialize JSON patch.");
		return;
	}

	RING_API_RETSTRING(patchJson.c_str());
}

// ============================================================================
// JSON Merge Patch (RFC 7386)
// ============================================================================

RING_FUNC(ring_json_merge_patch)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cTargetJson = RING_API_GETSTRING(1);
	const char *cPatchJson = RING_API_GETSTRING(2);

	auto result = glz::merge_patch_json(std::string_view(cTargetJson), std::string_view(cPatchJson));
	if (!result)
	{
		RING_API_ERROR("Failed to apply merge patch.");
		return;
	}

	RING_API_RETSTRING(result->c_str());
}

RING_FUNC(ring_json_merge_diff)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cSourceJson = RING_API_GETSTRING(1);
	const char *cTargetJson = RING_API_GETSTRING(2);

	auto patch = glz::merge_diff_json(std::string_view(cSourceJson), std::string_view(cTargetJson));
	if (!patch)
	{
		RING_API_ERROR("Failed to generate merge diff.");
		return;
	}

	RING_API_RETSTRING(patch->c_str());
}

// ============================================================================
// CBOR (Concise Binary Object Representation)
// ============================================================================

RING_FUNC(ring_json_to_cbor)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	std::string cborData;
	auto ec2 = glz::write_cbor(jsonRoot, cborData);
	if (ec2)
	{
		RING_API_ERROR("Failed to convert to CBOR.");
		return;
	}

	RING_API_RETSTRING2(cborData.c_str(), cborData.size());
}

RING_FUNC(ring_cbor_to_json)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cCborData = RING_API_GETSTRING(1);
	int nSize = RING_API_GETSTRINGSIZE(1);

	glz::generic jsonRoot;
	auto ec = glz::read_cbor(jsonRoot, std::string_view(cCborData, nSize));
	if (ec)
	{
		RING_API_ERROR("Invalid CBOR input.");
		return;
	}

	std::string jsonString;
	auto ec2 = glz::write_json(jsonRoot, jsonString);
	if (ec2)
	{
		RING_API_ERROR("Failed to convert to JSON.");
		return;
	}

	RING_API_RETSTRING(jsonString.c_str());
}

// ============================================================================
// NDJSON (Newline Delimited JSON / JSON Lines)
// ============================================================================

RING_FUNC(ring_json_ndjson_encode)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISLIST(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	if (RING_API_ISOBJECT(1))
	{
		RING_API_ERROR("Cannot encode a Ring object to NDJSON.");
		return;
	}

	List *pList = RING_API_GETLIST(1);
	VM *pVM = static_cast<VM *>(pPointer);

	// Build array of generic values
	std::vector<glz::generic> items;
	items.reserve(ring_list_getsize(pList));

	List *pVisited = ring_list_new_gc(pVM->pRingState, 0);

	for (int i = 1; i <= ring_list_getsize(pList); i++)
	{
		Item *pItem = ring_list_getitem(pList, i);
		items.push_back(ring_item_to_json_value(pVM, pItem, pVisited));
		// Clear visited for next item
		ring_list_deleteallitems_gc(pVM->pRingState, pVisited);
	}

	ring_list_delete_gc(pVM->pRingState, pVisited);

	// Write as NDJSON
	std::string ndjsonResult;
	auto ec = glz::write_ndjson(items, ndjsonResult);
	if (ec)
	{
		RING_API_ERROR("Failed to encode NDJSON.");
		return;
	}

	RING_API_RETSTRING(ndjsonResult.c_str());
}

RING_FUNC(ring_json_ndjson_decode)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cNdjsonString = RING_API_GETSTRING(1);
	std::string ndjsonInput(cNdjsonString);

	std::vector<glz::generic> items;
	auto ec = glz::read_ndjson(items, ndjsonInput);
	if (ec)
	{
		RING_API_ERROR("Failed to decode NDJSON.");
		return;
	}

	// Convert to Ring list
	List *pResultList = RING_API_NEWLIST;
	for (const auto &item : items)
	{
		List *pTempList = ring_list_newlist_gc(RING_API_STATE, pResultList);
		json_value_to_ring_item(RING_API_STATE, item, pTempList);

		// Unwrap single-item sublists
		if (ring_list_getsize(pTempList) == 1)
		{
			if (ring_list_islist(pTempList, 1))
			{
				List *pInner = ring_list_getlist(pTempList, 1);
				ring_list_swaptwolists(pTempList, pInner);
			}
		}
	}

	RING_API_RETLISTBYREF(pResultList);
}

// ============================================================================
// Utility Functions
// ============================================================================

// Helper: Compare two glz::generic values for equality
static bool generic_equal(const glz::generic &a, const glz::generic &b)
{
	// Check if same type
	if (a.data.index() != b.data.index())
	{
		return false;
	}

	if (auto *obj_a = a.get_if<glz::generic::object_t>())
	{
		auto *obj_b = b.get_if<glz::generic::object_t>();
		if (!obj_b || obj_a->size() != obj_b->size())
			return false;
		for (const auto &[key, val] : *obj_a)
		{
			auto it = obj_b->find(key);
			if (it == obj_b->end() || !generic_equal(val, it->second))
				return false;
		}
		return true;
	}
	else if (auto *arr_a = a.get_if<glz::generic::array_t>())
	{
		auto *arr_b = b.get_if<glz::generic::array_t>();
		if (!arr_b || arr_a->size() != arr_b->size())
			return false;
		for (size_t i = 0; i < arr_a->size(); ++i)
		{
			if (!generic_equal((*arr_a)[i], (*arr_b)[i]))
				return false;
		}
		return true;
	}
	else if (auto *str_a = a.get_if<std::string>())
	{
		auto *str_b = b.get_if<std::string>();
		return str_b && *str_a == *str_b;
	}
	else if (auto *num_a = a.get_if<double>())
	{
		auto *num_b = b.get_if<double>();
		return num_b && *num_a == *num_b;
	}
	else if (auto *bool_a = a.get_if<bool>())
	{
		auto *bool_b = b.get_if<bool>();
		return bool_b && *bool_a == *bool_b;
	}
	else
	{
		// Both are null
		return true;
	}
}

// Helper: Count elements in a glz::generic value
static size_t generic_size(const glz::generic &value)
{
	if (auto *obj = value.get_if<glz::generic::object_t>())
	{
		return obj->size();
	}
	else if (auto *arr = value.get_if<glz::generic::array_t>())
	{
		return arr->size();
	}
	return 1; // Scalar values count as 1
}

// Helper: Calculate max depth of a glz::generic value
static size_t generic_depth(const glz::generic &value, size_t current = 0)
{
	if (auto *obj = value.get_if<glz::generic::object_t>())
	{
		size_t max_depth = current + 1;
		for (const auto &[key, val] : *obj)
		{
			max_depth = std::max(max_depth, generic_depth(val, current + 1));
		}
		return max_depth;
	}
	else if (auto *arr = value.get_if<glz::generic::array_t>())
	{
		size_t max_depth = current + 1;
		for (const auto &val : *arr)
		{
			max_depth = std::max(max_depth, generic_depth(val, current + 1));
		}
		return max_depth;
	}
	return current;
}

// Helper: Sort keys in a glz::generic object (recursively)
static glz::generic generic_sort_keys(const glz::generic &value)
{
	if (auto *obj = value.get_if<glz::generic::object_t>())
	{
		// Collect and sort keys
		std::vector<std::string> keys;
		keys.reserve(obj->size());
		for (const auto &[key, val] : *obj)
		{
			keys.push_back(key);
		}
		std::sort(keys.begin(), keys.end());

		// Build new sorted object
		glz::generic::object_t sorted_obj;
		for (const auto &key : keys)
		{
			sorted_obj[key] = generic_sort_keys(obj->at(key));
		}

		glz::generic result;
		result.data = std::move(sorted_obj);
		return result;
	}
	else if (auto *arr = value.get_if<glz::generic::array_t>())
	{
		glz::generic::array_t sorted_arr;
		sorted_arr.reserve(arr->size());
		for (const auto &val : *arr)
		{
			sorted_arr.push_back(generic_sort_keys(val));
		}

		glz::generic result;
		result.data = std::move(sorted_arr);
		return result;
	}

	return value;
}

RING_FUNC(ring_json_equal)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJson1 = RING_API_GETSTRING(1);
	const char *cJson2 = RING_API_GETSTRING(2);

	glz::generic json1, json2;

	auto ec1 = glz::read_json(json1, std::string_view(cJson1));
	if (ec1)
	{
		RING_API_RETNUMBER(0.0);
		return;
	}

	auto ec2 = glz::read_json(json2, std::string_view(cJson2));
	if (ec2)
	{
		RING_API_RETNUMBER(0.0);
		return;
	}

	RING_API_RETNUMBER(generic_equal(json1, json2) ? 1.0 : 0.0);
}

RING_FUNC(ring_json_size)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	RING_API_RETNUMBER(static_cast<double>(generic_size(jsonRoot)));
}

RING_FUNC(ring_json_depth)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	RING_API_RETNUMBER(static_cast<double>(generic_depth(jsonRoot)));
}

RING_FUNC(ring_json_sort_keys)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	glz::generic sorted = generic_sort_keys(jsonRoot);

	std::string result;
	auto ec2 = glz::write_json(sorted, result);
	if (ec2)
	{
		RING_API_ERROR("Failed to serialize sorted JSON.");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

// ============================================================================
// Base64 Encoding/Decoding
// ============================================================================

RING_FUNC(ring_base64_encode)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cData = RING_API_GETSTRING(1);
	int nSize = RING_API_GETSTRINGSIZE(1);

	std::string encoded = glz::write_base64(std::string_view(cData, nSize));
	RING_API_RETSTRING(encoded.c_str());
}

RING_FUNC(ring_base64_decode)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cEncoded = RING_API_GETSTRING(1);
	std::string decoded = glz::read_base64(std::string_view(cEncoded));
	RING_API_RETSTRING2(decoded.c_str(), decoded.size());
}

// ============================================================================
// JSON with Comments (JSONC)
// ============================================================================

RING_FUNC(ring_json_strip_comments)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string input(cJsonString);

	// Parse with comments enabled
	glz::generic jsonRoot;
	auto ec = glz::read<glz::opts{.comments = true}>(jsonRoot, input);
	if (ec)
	{
		std::string errorMsg = "JSON error: " + std::string(glz::format_error(ec, input));
		RING_API_ERROR(errorMsg.c_str());
		return;
	}

	// Write back as standard JSON (comments stripped)
	std::string result;
	auto ec2 = glz::write_json(jsonRoot, result);
	if (ec2)
	{
		RING_API_ERROR("Failed to serialize JSON.");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

// ============================================================================
// JSON Query (simplified path-based query)
// ============================================================================

RING_FUNC(ring_json_query)
{
	if (RING_API_PARACOUNT != 2)
	{
		RING_API_ERROR(RING_API_MISS2PARA);
		return;
	}

	if (!RING_API_ISSTRING(1) || !RING_API_ISSTRING(2))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	const char *cQuery = RING_API_GETSTRING(2);

	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	std::string query(cQuery);
	glz::generic *current = &jsonRoot;

	if (!query.empty() && query[0] == '/')
	{
		query = query.substr(1);
	}

	std::vector<std::string> tokens;
	size_t pos = 0;
	while ((pos = query.find('/')) != std::string::npos)
	{
		tokens.push_back(query.substr(0, pos));
		query.erase(0, pos + 1);
	}
	if (!query.empty())
	{
		tokens.push_back(query);
	}

	for (const auto &token : tokens)
	{
		if (auto *obj = current->get_if<glz::generic::object_t>())
		{
			auto it = obj->find(token);
			if (it == obj->end())
			{
				RING_API_RETSTRING("null");
				return;
			}
			current = &(it->second);
		}
		else if (auto *arr = current->get_if<glz::generic::array_t>())
		{
			try
			{
				size_t index = std::stoull(token);
				if (index >= arr->size())
				{
					RING_API_RETSTRING("null");
					return;
				}
				current = &((*arr)[index]);
			}
			catch (...)
			{
				RING_API_RETSTRING("null");
				return;
			}
		}
		else
		{
			RING_API_RETSTRING("null");
			return;
		}
	}

	std::string result;
	auto ec2 = glz::write_json(*current, result);
	if (ec2)
	{
		RING_API_RETSTRING("null");
		return;
	}

	RING_API_RETSTRING(result.c_str());
}

// ============================================================================
// CSV Support
// ============================================================================

RING_FUNC(ring_csv_to_json)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cCsvString = RING_API_GETSTRING(1);
	std::string csvInput(cCsvString);

	// Parse CSV into vector of vectors (rows of values)
	std::vector<std::vector<std::string>> rows;
	std::istringstream stream(csvInput);
	std::string line;

	while (std::getline(stream, line))
	{
		if (line.empty())
			continue;

		std::vector<std::string> row;
		std::string cell;
		bool inQuotes = false;

		for (size_t i = 0; i < line.size(); ++i)
		{
			char c = line[i];
			if (c == '"')
			{
				inQuotes = !inQuotes;
			}
			else if (c == ',' && !inQuotes)
			{
				row.push_back(cell);
				cell.clear();
			}
			else
			{
				cell += c;
			}
		}
		row.push_back(cell);
		rows.push_back(row);
	}

	if (rows.empty())
	{
		RING_API_RETSTRING("[]");
		return;
	}

	// First row is headers
	std::vector<std::string> headers = rows[0];

	// Build JSON array of objects
	glz::generic::array_t jsonArray;

	for (size_t i = 1; i < rows.size(); ++i)
	{
		glz::generic::object_t obj;
		for (size_t j = 0; j < headers.size() && j < rows[i].size(); ++j)
		{
			const std::string &val = rows[i][j];
			glz::generic cellValue;

			try
			{
				size_t pos;
				double num = std::stod(val, &pos);
				if (pos == val.size())
				{
					cellValue.data = num;
				}
				else
				{
					cellValue.data = val;
				}
			}
			catch (...)
			{
				cellValue.data = val;
			}
			obj[headers[j]] = std::move(cellValue);
		}
		glz::generic rowObj;
		rowObj.data = std::move(obj);
		jsonArray.push_back(std::move(rowObj));
	}

	glz::generic result;
	result.data = std::move(jsonArray);

	std::string jsonOutput;
	auto ec = glz::write_json(result, jsonOutput);
	if (ec)
	{
		RING_API_ERROR("Failed to serialize JSON.");
		return;
	}

	RING_API_RETSTRING(jsonOutput.c_str());
}

RING_FUNC(ring_json_to_csv)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	auto *arr = jsonRoot.get_if<glz::generic::array_t>();
	if (!arr || arr->empty())
	{
		RING_API_ERROR("JSON must be a non-empty array of objects.");
		return;
	}

	// Get headers from first object
	auto *firstObj = (*arr)[0].get_if<glz::generic::object_t>();
	if (!firstObj)
	{
		RING_API_ERROR("JSON array must contain objects.");
		return;
	}

	std::vector<std::string> headers;
	for (const auto &[key, val] : *firstObj)
	{
		headers.push_back(key);
	}
	std::sort(headers.begin(), headers.end());

	std::ostringstream csv;

	// Write headers
	for (size_t i = 0; i < headers.size(); ++i)
	{
		if (i > 0)
			csv << ",";
		csv << headers[i];
	}
	csv << "\n";

	// Write rows
	for (const auto &item : *arr)
	{
		auto *obj = item.get_if<glz::generic::object_t>();
		if (!obj)
			continue;

		for (size_t i = 0; i < headers.size(); ++i)
		{
			if (i > 0)
				csv << ",";

			auto it = obj->find(headers[i]);
			if (it != obj->end())
			{
				const auto &val = it->second;
				if (auto *str = val.get_if<std::string>())
				{
					// Escape quotes and wrap in quotes if contains comma
					if (str->find(',') != std::string::npos || str->find('"') != std::string::npos)
					{
						csv << "\"";
						for (char c : *str)
						{
							if (c == '"')
								csv << "\"\"";
							else
								csv << c;
						}
						csv << "\"";
					}
					else
					{
						csv << *str;
					}
				}
				else if (auto *num = val.get_if<double>())
				{
					csv << *num;
				}
				else if (auto *b = val.get_if<bool>())
				{
					csv << (*b ? "true" : "false");
				}
				// null and complex types are left empty
			}
		}
		csv << "\n";
	}

	RING_API_RETSTRING(csv.str().c_str());
}

// ============================================================================
// TOML Support
// ============================================================================

RING_FUNC(ring_toml_to_json)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cTomlString = RING_API_GETSTRING(1);
	std::string tomlInput(cTomlString);

	glz::generic tomlRoot;
	auto ec = glz::read<glz::opts{.format = glz::TOML}>(tomlRoot, tomlInput);
	if (ec)
	{
		std::string errorMsg = "TOML error: " + std::string(glz::format_error(ec, tomlInput));
		RING_API_ERROR(errorMsg.c_str());
		return;
	}

	std::string jsonOutput;
	auto ec2 = glz::write_json(tomlRoot, jsonOutput);
	if (ec2)
	{
		RING_API_ERROR("Failed to serialize JSON.");
		return;
	}

	RING_API_RETSTRING(jsonOutput.c_str());
}

RING_FUNC(ring_json_to_toml)
{
	if (RING_API_PARACOUNT != 1)
	{
		RING_API_ERROR(RING_API_MISS1PARA);
		return;
	}

	if (!RING_API_ISSTRING(1))
	{
		RING_API_ERROR(RING_API_BADPARATYPE);
		return;
	}

	const char *cJsonString = RING_API_GETSTRING(1);
	std::string_view jsonView(cJsonString);

	glz::generic jsonRoot;
	auto ec = glz::read_json(jsonRoot, jsonView);
	if (ec)
	{
		RING_API_ERROR("Invalid JSON input.");
		return;
	}

	std::string tomlOutput;
	auto ec2 = glz::write<glz::opts{.format = glz::TOML}>(jsonRoot, tomlOutput);
	if (ec2)
	{
		RING_API_ERROR("Failed to serialize TOML.");
		return;
	}

	RING_API_RETSTRING(tomlOutput.c_str());
}

RING_LIBINIT
{
	RING_API_REGISTER("json_decode", ring_json_decode);
	RING_API_REGISTER("json_encode", ring_json_encode);
	RING_API_REGISTER("json_version", ring_json_version);
	RING_API_REGISTER("json_valid", ring_json_valid);
	RING_API_REGISTER("json_minify", ring_json_minify);
	RING_API_REGISTER("json_prettify", ring_json_prettify);
	RING_API_REGISTER("json_pointer", ring_json_pointer);
	RING_API_REGISTER("json_type", ring_json_type);
	RING_API_REGISTER("json_keys", ring_json_keys);
	RING_API_REGISTER("json_has", ring_json_has);
	RING_API_REGISTER("json_merge", ring_json_merge);
	RING_API_REGISTER("json_read_file", ring_json_read_file);
	RING_API_REGISTER("json_write_file", ring_json_write_file);
	RING_API_REGISTER("json_to_beve", ring_json_to_beve);
	RING_API_REGISTER("beve_to_json", ring_beve_to_json);
	RING_API_REGISTER("json_patch", ring_json_patch);
	RING_API_REGISTER("json_diff", ring_json_diff);
	RING_API_REGISTER("json_merge_patch", ring_json_merge_patch);
	RING_API_REGISTER("json_merge_diff", ring_json_merge_diff);
	RING_API_REGISTER("json_to_cbor", ring_json_to_cbor);
	RING_API_REGISTER("cbor_to_json", ring_cbor_to_json);
	RING_API_REGISTER("json_ndjson_encode", ring_json_ndjson_encode);
	RING_API_REGISTER("json_ndjson_decode", ring_json_ndjson_decode);
	RING_API_REGISTER("json_equal", ring_json_equal);
	RING_API_REGISTER("json_size", ring_json_size);
	RING_API_REGISTER("json_depth", ring_json_depth);
	RING_API_REGISTER("json_sort_keys", ring_json_sort_keys);
	RING_API_REGISTER("base64_encode", ring_base64_encode);
	RING_API_REGISTER("base64_decode", ring_base64_decode);
	RING_API_REGISTER("json_strip_comments", ring_json_strip_comments);
	RING_API_REGISTER("json_query", ring_json_query);
	RING_API_REGISTER("csv_to_json", ring_csv_to_json);
	RING_API_REGISTER("json_to_csv", ring_json_to_csv);
	RING_API_REGISTER("toml_to_json", ring_toml_to_json);
	RING_API_REGISTER("json_to_toml", ring_json_to_toml);
}
