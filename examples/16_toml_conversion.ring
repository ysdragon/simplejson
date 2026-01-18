load "simplejson.ring"

? "=== TOML Conversion ==="
? ""

? "--- toml_to_json() ---"
toml = 'title = "My Application"
version = "1.0.0"

[database]
host = "localhost"
port = 5432
name = "mydb"

[server]
host = "0.0.0.0"
port = 8080
debug = true'

? "Input TOML:"
? toml
? ""

json = toml_to_json(toml)
? "Output JSON:"
? json_prettify(json)
? ""

? "--- json_to_toml() ---"
jsonData = '{
    "app": "MyApp",
    "port": 3000,
    "debug": false,
    "features": {
        "logging": true,
        "metrics": false
    }
}'

? "Input JSON:"
? jsonData
? ""

tomlOutput = json_to_toml(jsonData)
? "Output TOML:"
? tomlOutput
? ""

? "Done!"
