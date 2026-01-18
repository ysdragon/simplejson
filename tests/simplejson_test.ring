load "stdlibcore.ring"

arch = getarch()
osDir = ""
archDir = ""
libName = ""
libVariant = ""

if isWindows()
	osDir = "windows"
	libName = "ring_simplejson.dll"
	if arch = "x64"
		archDir = "amd64"
	but arch = "arm64"
		archDir = "arm64"
	but arch = "x86"
		archDir = "i386"
	else
		raise("Unsupported Windows architecture: " + arch)
	ok
but isLinux()
	osDir = "linux"
	libName = "libring_simplejson.so"
	if arch = "x64"
		archDir = "amd64"
	but arch = "arm64"
		archDir = "arm64"
	else
		raise("Unsupported Linux architecture: " + arch)
	ok
	if isMusl()
		libVariant = "musl/"
	ok
but isFreeBSD()
	osDir = "freebsd"
	libName = "libring_simplejson.so"
	if arch = "x64"
		archDir = "amd64"
	but arch = "arm64"
		archDir = "arm64"
	else
		raise("Unsupported FreeBSD architecture: " + arch)
	ok
but isMacOSX()
	osDir = "macos"
	libName = "libring_simplejson.dylib"
	if arch = "x64"
		archDir = "amd64"
	but arch = "arm64"
		archDir = "arm64"
	else
		raise("Unsupported macOS architecture: " + arch)
	ok
else
	raise("Unsupported OS! You need to build the library for your OS.")
ok

loadlib("../lib/" + osDir + "/" + libVariant + archDir + "/" + libName)

func main
	new SimpleJSONTest()

func isMusl
	cOutput = systemCmd("sh -c 'ldd 2>&1'")
	return substr(cOutput, "musl") > 0

class SimpleJSONTest

	cTestDir = "test_data"
	nTestsRun = 0
	nTestsFailed = 0

	func init
		? "Setting up test environment..."
		setupTestData()
		? "Test environment ready." + nl
		runAllTests()

	func setupTestData
		if isWindows()
			system("rmdir /s /q " + cTestDir + " 2>nul")
			system("mkdir " + cTestDir)
		else
			system("rm -rf " + cTestDir + " 2>/dev/null")
			system("mkdir -p " + cTestDir)
		ok
		write(cTestDir + "/test.json", '{"name":"Test","value":123}')
		write(cTestDir + "/test.csv", "name,age,city" + nl + "Alice,30,NYC" + nl + "Bob,25,LA" + nl)
		write(cTestDir + "/test.toml", 'title = "Test"' + nl + "[settings]" + nl + "enabled = true" + nl)

	func cleanup
		if isWindows()
			system("rmdir /s /q " + cTestDir + " 2>nul")
		else
			system("rm -rf " + cTestDir + " 2>/dev/null")
		ok

	func assert(condition, message)
		if !condition
			raise("Assertion Failed: " + message)
		ok

	func assertEqual(actual, expected, message)
		if actual != expected
			raise(message + ": expected '" + expected + "' got '" + actual + "'")
		ok

	func run(testName, methodName)
		nTestsRun++
		see "  " + testName + "..."
		try
			call methodName()
			see " [PASS]" + nl
		catch
			nTestsFailed++
			see " [FAIL]" + nl
			see "    -> " + cCatchError + nl
		done

	func runAllTests
		? "========================================"
		? "  Running SimpleJSON Test Suite"
		? "========================================" + nl

		? "Core Functions:"
		run("json_decode", :testJsonDecode)
		run("json_encode", :testJsonEncode)
		run("json_version", :testJsonVersion)
		run("json_valid", :testJsonValid)
		? ""

		? "String Formatting:"
		run("json_minify", :testJsonMinify)
		run("json_prettify", :testJsonPrettify)
		? ""

		? "Object Manipulation:"
		run("json_type", :testJsonType)
		run("json_keys", :testJsonKeys)
		run("json_has", :testJsonHas)
		run("json_merge", :testJsonMerge)
		run("json_pointer", :testJsonPointer)
		? ""

		? "Utility Functions:"
		run("json_equal", :testJsonEqual)
		run("json_size", :testJsonSize)
		run("json_depth", :testJsonDepth)
		run("json_sort_keys", :testJsonSortKeys)
		run("json_query", :testJsonQuery)
		? ""

		? "File I/O:"
		run("json_read_file", :testJsonReadFile)
		run("json_write_file", :testJsonWriteFile)
		? ""

		? "Binary Formats:"
		run("json_to_beve / beve_to_json", :testBeve)
		run("json_to_cbor / cbor_to_json", :testCbor)
		? ""

		? "JSON Patch (RFC 6902):"
		run("json_patch", :testJsonPatch)
		run("json_diff", :testJsonDiff)
		? ""

		? "JSON Merge Patch (RFC 7386):"
		run("json_merge_patch", :testJsonMergePatch)
		run("json_merge_diff", :testJsonMergeDiff)
		? ""

		? "NDJSON:"
		run("json_ndjson_encode", :testNdjsonEncode)
		run("json_ndjson_decode", :testNdjsonDecode)
		? ""

		? "Base64:"
		run("base64_encode", :testBase64Encode)
		run("base64_decode", :testBase64Decode)
		? ""

		? "Comments (JSONC):"
		run("json_strip_comments", :testJsonStripComments)
		? ""

		? "CSV Support:"
		run("csv_to_json", :testCsvToJson)
		run("json_to_csv", :testJsonToCsv)
		? ""

		? "TOML Support:"
		run("toml_to_json", :testTomlToJson)
		run("json_to_toml", :testJsonToToml)
		? ""

		cleanup()

		? "========================================"
		? "  Test Results: " + (nTestsRun - nTestsFailed) + "/" + nTestsRun + " passed"
		? "========================================"

		if nTestsFailed > 0
			raise("" + nTestsFailed + " test(s) failed!")
		ok

	func testJsonDecode
		data = json_decode('{"name":"John","age":30}')
		assertEqual(data[:name], "John", "name field")
		assertEqual(data[:age], 30, "age field")

		arr = json_decode('[1,2,3]')
		assertEqual(len(arr), 3, "array length")
		assertEqual(arr[1], 1, "first element")

	func testJsonEncode
		data = [["name", "John"], ["age", 30]]
		result = json_encode(data)
		assert(substr(result, '"name"') > 0, "should contain name")
		assert(substr(result, '"age"') > 0, "should contain age")

	func testJsonVersion
		ver = json_version()
		assert(len(ver) > 0, "version should not be empty")
		assert(substr(ver, ".") > 0, "version should contain dot")

	func testJsonValid
		assertEqual(json_valid('{"a":1}'), 1, "valid JSON")
		assertEqual(json_valid('{invalid}'), 0, "invalid JSON")
		assertEqual(json_valid('[1,2,3]'), 1, "valid array")

	func testJsonMinify
		result = json_minify('{  "a" :  1  }')
		assertEqual(result, '{"a":1}', "minified JSON")

	func testJsonPrettify
		result = json_prettify('{"a":1}')
		assert(substr(result, nl) > 0, "should contain newlines")

	func testJsonType
		assertEqual(json_type('{"a":1}'), "object", "object type")
		assertEqual(json_type('[1,2,3]'), "array", "array type")
		assertEqual(json_type('"hello"'), "string", "string type")
		assertEqual(json_type('42'), "number", "number type")
		assertEqual(json_type('true'), "boolean", "boolean type")
		assertEqual(json_type('null'), "null", "null type")

	func testJsonKeys
		keys = json_keys('{"a":1,"b":2,"c":3}')
		assertEqual(len(keys), 3, "should have 3 keys")

	func testJsonHas
		json = '{"name":"John","age":30}'
		assertEqual(json_has(json, "name"), 1, "should have name")
		assertEqual(json_has(json, "email"), 0, "should not have email")

	func testJsonMerge
		result = json_merge('{"a":1}', '{"b":2}')
		assert(substr(result, '"a"') > 0, "should contain a")
		assert(substr(result, '"b"') > 0, "should contain b")

	func testJsonPointer
		json = '{"user":{"name":"John"},"items":[1,2,3]}'
		assertEqual(json_pointer(json, "/user/name"), "John", "nested object")
		assertEqual(json_pointer(json, "/items/0"), 1, "array index")

	func testJsonEqual
		assertEqual(json_equal('{"a":1,"b":2}', '{"b":2,"a":1}'), 1, "same objects")
		assertEqual(json_equal('{"a":1}', '{"a":2}'), 0, "different values")
		assertEqual(json_equal('[1,2,3]', '[1,2,3]'), 1, "same arrays")

	func testJsonSize
		assertEqual(json_size('{"a":1,"b":2,"c":3}'), 3, "object size")
		assertEqual(json_size('[1,2,3,4,5]'), 5, "array size")
		assertEqual(json_size('{}'), 0, "empty object")

	func testJsonDepth
		assertEqual(json_depth('{"a":1}'), 1, "flat object")
		assertEqual(json_depth('{"a":{"b":1}}'), 2, "nested object")
		assertEqual(json_depth('{"a":{"b":{"c":1}}}'), 3, "deep nested")

	func testJsonSortKeys
		result = json_sort_keys('{"z":1,"a":2,"m":3}')
		assert(substr(result, '"a"') < substr(result, '"m"'), "a before m")
		assert(substr(result, '"m"') < substr(result, '"z"'), "m before z")

	func testJsonQuery
		json = '{"users":[{"name":"Alice"},{"name":"Bob"}]}'
		assertEqual(json_query(json, "/users/0/name"), '"Alice"', "query nested")
		assertEqual(json_query(json, "/nonexistent"), "null", "query missing")

	func testJsonReadFile
		data = json_read_file(cTestDir + "/test.json")
		assertEqual(data[:name], "Test", "read name")
		assertEqual(data[:value], 123, "read value")

	func testJsonWriteFile
		data = [["test", "value"]]
		result = json_write_file(data, cTestDir + "/output.json", 0)
		assertEqual(result, 1, "write success")
		content = read(cTestDir + "/output.json")
		assert(substr(content, '"test"') > 0, "file should contain test")

	func testBeve
		json = '{"test":123}'
		beve = json_to_beve(json)
		assert(len(beve) > 0, "beve should not be empty")
		restored = beve_to_json(beve)
		assert(substr(restored, '"test"') > 0, "restored should contain test")

	func testCbor
		json = '{"test":123}'
		cbor = json_to_cbor(json)
		assert(len(cbor) > 0, "cbor should not be empty")
		restored = cbor_to_json(cbor)
		assert(substr(restored, '"test"') > 0, "restored should contain test")

	func testJsonPatch
		doc = '{"name":"John"}'
		ops = '[{"op":"replace","path":"/name","value":"Jane"}]'
		result = json_patch(doc, ops)
		assert(substr(result, '"Jane"') > 0, "should contain Jane")

	func testJsonDiff
		source = '{"a":1}'
		target = '{"a":2}'
		patch = json_diff(source, target)
		assert(substr(patch, '"op"') > 0, "should contain op")

	func testJsonMergePatch
		target = '{"a":"b","c":"d"}'
		patch = '{"c":"e"}'
		result = json_merge_patch(target, patch)
		assert(substr(result, '"e"') > 0, "should contain e")

	func testJsonMergeDiff
		source = '{"a":1}'
		target = '{"a":2}'
		patch = json_merge_diff(source, target)
		assert(substr(patch, '"a"') > 0, "should contain a")

	func testNdjsonEncode
		records = [[["id", 1]], [["id", 2]]]
		result = json_ndjson_encode(records)
		assert(substr(result, nl) > 0, "should contain newlines")

	func testNdjsonDecode
		ndjson = '{"a":1}' + nl + '{"b":2}'
		records = json_ndjson_decode(ndjson)
		assertEqual(len(records), 2, "should have 2 records")

	func testBase64Encode
		result = base64_encode("Hello")
		assertEqual(result, "SGVsbG8=", "base64 encode")

	func testBase64Decode
		result = base64_decode("SGVsbG8=")
		assertEqual(result, "Hello", "base64 decode")

	func testJsonStripComments
		jsonc = '{"a":1 /* comment */}'
		result = json_strip_comments(jsonc)
		assertEqual(result, '{"a":1}', "strip comments")

	func testCsvToJson
		csv = "name,age" + nl + "Alice,30" + nl
		result = csv_to_json(csv)
		assert(substr(result, '"Alice"') > 0, "should contain Alice")

	func testJsonToCsv
		json = '[{"name":"Alice","age":30}]'
		result = json_to_csv(json)
		assert(substr(result, "name") > 0, "should contain header")
		assert(substr(result, "Alice") > 0, "should contain Alice")

	func testTomlToJson
		toml = 'title = "Test"'
		result = toml_to_json(toml)
		assert(substr(result, '"title"') > 0, "should contain title")
		assert(substr(result, '"Test"') > 0, "should contain Test")

	func testJsonToToml
		json = '{"name":"Test"}'
		result = json_to_toml(json)
		assert(substr(result, 'name = "Test"') > 0, "should contain name = Test")
