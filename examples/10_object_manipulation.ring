load "simplejson.ring"

? "=== Object Manipulation ==="
? ""

json = '{"name":"John","age":30,"city":"NYC"}'

? "--- json_keys() ---"
keys = json_keys(json)
? "Keys in object:"
for key in keys
    ? "  - " + key
next
? ""

? "--- json_has() ---"
? 'Has "name": ' + json_has(json, "name")
? 'Has "email": ' + json_has(json, "email")
? ""

? "--- json_merge() ---"
obj1 = '{"a":1,"b":2}'
obj2 = '{"b":3,"c":4}'
? "Object 1: " + obj1
? "Object 2: " + obj2
? "Merged: " + json_merge(obj1, obj2)
? ""

? "--- json_equal() ---"
? 'Equal (same keys, diff order): ' + json_equal('{"a":1,"b":2}', '{"b":2,"a":1}')
? 'Equal (different values): ' + json_equal('{"a":1}', '{"a":2}')
? 'Equal (same arrays): ' + json_equal('[1,2,3]', '[1,2,3]')
? ""

? "Done!"
