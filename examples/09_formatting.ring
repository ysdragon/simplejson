load "simplejson.ring"

? "=== JSON Formatting ==="
? ""

json = '{"name":"John","age":30,"city":"NYC"}'

? "--- Original ---"
? json
? ""

? "--- json_prettify() ---"
? json_prettify(json)
? ""

? "--- json_minify() ---"
pretty = '{
    "name": "John",
    "age": 30,
    "city": "NYC"
}'
? "Input (pretty):"
? pretty
? ""
? "Output (minified):"
? json_minify(pretty)
? ""

? "--- json_sort_keys() ---"
unsorted = '{"z":1,"a":2,"m":3,"b":{"y":1,"x":2}}'
? "Unsorted: " + unsorted
? "Sorted: " + json_sort_keys(unsorted)
? ""

? "Done!"
