load "simplejson.ring"

? "=== JSON Validation & Information ==="
? ""

? "--- json_version() ---"
? "Glaze version: " + json_version()
? ""

? "--- json_valid() ---"
? "Valid JSON: " + json_valid('{"name": "John"}')
? "Invalid JSON: " + json_valid('{invalid}')
? "Valid array: " + json_valid('[1, 2, 3]')
? ""

? "--- json_type() ---"
? 'Type of {"a":1}: ' + json_type('{"a":1}')
? 'Type of [1,2,3]: ' + json_type('[1,2,3]')
? 'Type of "hello": ' + json_type('"hello"')
? 'Type of 42: ' + json_type('42')
? 'Type of true: ' + json_type('true')
? 'Type of null: ' + json_type('null')
? ""

? "--- json_size() ---"
? "Object with 3 keys: " + json_size('{"a":1,"b":2,"c":3}')
? "Array with 5 elements: " + json_size('[1,2,3,4,5]')
? "Empty object: " + json_size('{}')
? ""

? "--- json_depth() ---"
? "Flat object: " + json_depth('{"a":1}')
? "Nested 2 levels: " + json_depth('{"a":{"b":1}}')
? "Nested 3 levels: " + json_depth('{"a":{"b":{"c":1}}}')
? "Array nested: " + json_depth('[[1,2],[3,4]]')
? ""

? "Done!"
