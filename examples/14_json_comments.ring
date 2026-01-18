load "simplejson.ring"

? "=== JSON with Comments (JSONC) ==="
? ""

? "--- json_strip_comments() ---"
? ""

jsonc1 = '{
    // This is a single-line comment
    "name": "MyApp",
    "version": "1.0"
}'
? "Input with single-line comments:"
? jsonc1
? ""
? "Output:"
? json_strip_comments(jsonc1)
? ""

jsonc2 = '{
    /* This is a
       multi-line comment */
    "enabled": true,
    "timeout": 30
}'
? "Input with multi-line comments:"
? jsonc2
? ""
? "Output:"
? json_strip_comments(jsonc2)
? ""

jsonc3 = '{
    // Database configuration
    "database": {
        "host": "localhost", // Default host
        "port": 5432,        /* PostgreSQL default */
        "name": "mydb"
    }
}'
? "Input with mixed comments:"
? jsonc3
? ""
? "Output (prettified):"
? json_prettify(json_strip_comments(jsonc3))
? ""

? "Done!"
