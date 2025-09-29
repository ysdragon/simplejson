#include "jansson.h"
#include "ring.h"

#define IS_JSON_EMPTY_OBJECT(pList)                                                                                    \
	(ring_list_getsize(pList) == 1 && ring_list_isstring(pList, 1) &&                                                  \
	 strcmp(ring_list_getstring(pList, 1), "__JSON_EMPTY_OBJECT__") == 0)

static json_t *ring_item_to_json_value(VM *pVM, Item *pItem, List *pVisited);
static json_t *ring_list_to_json_value(VM *pVM, List *pList, List *pVisited);

static void json_value_to_ring_list(void *pState, json_t *pValue, List *pList)
{
	switch (json_typeof(pValue))
	{
	case JSON_OBJECT: {

		List *pObjectList = ring_list_newlist_gc(pState, pList);
		const char *key;
		json_t *value;

		json_object_foreach(pValue, key, value)
		{
			List *pItemList = ring_list_newlist_gc(pState, pObjectList);
			ring_list_addstring_gc(pState, pItemList, key);
			json_value_to_ring_list(pState, value, pItemList);
		}
		break;
	}
	case JSON_ARRAY: {

		List *pArrayList = ring_list_newlist_gc(pState, pList);
		size_t index;
		json_t *value;

		json_array_foreach(pValue, index, value)
		{
			json_value_to_ring_list(pState, value, pArrayList);
		}
		break;
	}
	case JSON_STRING:
		ring_list_addstring_gc(pState, pList, json_string_value(pValue));
		break;
	case JSON_INTEGER:
		ring_list_adddouble_gc(pState, pList, (double)json_integer_value(pValue));
		break;
	case JSON_REAL:
		ring_list_adddouble_gc(pState, pList, json_real_value(pValue));
		break;
	case JSON_TRUE:
		ring_list_adddouble_gc(pState, pList, 1.0);
		break;
	case JSON_FALSE:
		ring_list_adddouble_gc(pState, pList, 0.0);
		break;
	case JSON_NULL:

		ring_list_addstring_gc(pState, pList, RING_CSTR_EMPTY);
		break;
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
	json_error_t error;
	json_t *pJsonRoot;

	pJsonRoot = json_loads(cJsonString, 0, &error);

	if (!pJsonRoot)
	{
		char cErrorMsg[RING_LARGEBUF];
		snprintf(cErrorMsg, sizeof(cErrorMsg), "JSON error on line %d: %s", error.line, error.text);
		RING_API_ERROR(cErrorMsg);
		return;
	}

	List *pTempList = RING_API_NEWLIST;
	json_value_to_ring_list(RING_API_STATE, pJsonRoot, pTempList);
	json_decref(pJsonRoot);

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

static json_t *ring_list_to_json_value(VM *pVM, List *pList, List *pVisited)
{

	if (ring_list_findpointer(pVisited, pList))
	{
		return json_null();
	}
	ring_list_addpointer_gc(pVM->pRingState, pVisited, pList);

	json_t *pResult;
	if (IS_JSON_EMPTY_OBJECT(pList))
	{
		pResult = json_object();
	}
	else if (is_ring_list_a_json_object(pList))
	{
		pResult = json_object();
		for (int x = 1; x <= ring_list_getsize(pList); x++)
		{
			List *pSubList = ring_list_getlist(pList, x);
			const char *cKey = ring_list_getstring(pSubList, 1);
			Item *pValueItem = ring_list_getitem(pSubList, 2);
			json_t *pJsonValue = ring_item_to_json_value(pVM, pValueItem, pVisited);
			json_object_set_new(pResult, cKey, pJsonValue);
		}
	}
	else
	{
		pResult = json_array();
		for (int x = 1; x <= ring_list_getsize(pList); x++)
		{
			Item *pItem = ring_list_getitem(pList, x);
			json_t *pJsonValue = ring_item_to_json_value(pVM, pItem, pVisited);
			json_array_append_new(pResult, pJsonValue);
		}
	}

	ring_list_deletelastitem_gc(pVM->pRingState, pVisited);
	return pResult;
}

static json_t *ring_item_to_json_value(VM *pVM, Item *pItem, List *pVisited)
{
	switch (ring_item_gettype(pItem))
	{
	case ITEMTYPE_STRING: {
		String *pString = ring_item_getstring(pItem);
		if (ring_string_size(pString) == 0)
		{
			return json_null();
		}
		return json_string(ring_string_get(pString));
	}
	case ITEMTYPE_NUMBER: {
		double nNum = ring_item_getnumber(pItem);
		if (fmod(nNum, 1.0) == 0.0)
		{
			return json_integer((json_int_t)nNum);
		}
		else
		{
			return json_real(nNum);
		}
	}
	case ITEMTYPE_LIST:
		return ring_list_to_json_value(pVM, ring_item_getlist(pItem), pVisited);
	default:
		return json_null();
	}
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

	size_t nFlags = JSON_PRESERVE_ORDER;
	if (RING_API_PARACOUNT == 2)
	{
		if (RING_API_ISNUMBER(2) && RING_API_GETNUMBER(2) == 1.0)
		{
			nFlags |= JSON_INDENT(4) | JSON_SORT_KEYS;
		}
	}

	List *pList = RING_API_GETLIST(1);
	VM *pVM = (VM *)pPointer;

	List *pVisited = ring_list_new_gc(pVM->pRingState, 0);

	json_t *pJsonRoot = ring_list_to_json_value(pVM, pList, pVisited);
	ring_list_delete_gc(pVM->pRingState, pVisited);

	if (!pJsonRoot)
	{
		RING_API_ERROR("Failed to convert Ring list to JSON value.");
		return;
	}

	char *cJsonString = json_dumps(pJsonRoot, nFlags);

	if (cJsonString)
	{
		RING_API_RETSTRING(cJsonString);

		free(cJsonString);
	}
	else
	{
		RING_API_ERROR("Failed to generate JSON string from list.");
	}

	json_decref(pJsonRoot);
}

RING_FUNC(get_json_version)
{
	RING_API_RETSTRING(JANSSON_VERSION);
}

RING_LIBINIT
{
	RING_API_REGISTER("json_decode", ring_json_decode);
	RING_API_REGISTER("json_encode", ring_json_encode);
	RING_API_REGISTER("json_version", get_json_version);
}