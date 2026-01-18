load "simplejson.ring"

? "=== JSON File I/O ==="
? ""

cTestFile = "test_config.json"

? "--- json_write_file() ---"
config = [
    ["appName", "MyApp"],
    ["version", "1.0.0"],
    ["settings", [
        ["theme", "dark"],
        ["language", "en"],
        ["autoSave", 1]
    ]],
    ["users", ["admin", "guest"]]
]

result = json_write_file(config, cTestFile, 1)
? "Write result: " + result
? "File content:"
? read(cTestFile)
? ""

? "--- json_read_file() ---"
data = json_read_file(cTestFile)
? "App name: " + data[:appName]
? "Version: " + data[:version]
? "Theme: " + data[:settings][:theme]
? "First user: " + data[:users][1]
? ""

remove(cTestFile)
? "Done! (test file cleaned up)"
