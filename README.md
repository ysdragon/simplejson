<div align="center">

# Ring SimpleJSON

[license]: https://img.shields.io/github/license/ysdragon/simplejson?style=for-the-badge&logo=opensourcehardware&label=License&logoColor=C0CAF5&labelColor=414868&color=8c73cc
[language-ring]: https://img.shields.io/badge/language-Ring-2D54CB.svg?style=for-the-badge&labelColor=414868
[platform]: https://img.shields.io/badge/Platform-Windows%20|%20Linux%20|%20macOS%20|%20FreeBSD-8c73cc.svg?style=for-the-badge&labelColor=414868
[version]: https://img.shields.io/badge/dynamic/regex?url=https%3A%2F%2Fraw.githubusercontent.com%2Fysdragon%2Fsimplejson%2Fmaster%2Fpackage.ring&search=%3Aversion\s*%3D\s*"([^"]%2B)"&replace=%241&style=for-the-badge&label=version&labelColor=414868&color=7664C6

[![][license]](LICENSE)
[![][language-ring]](https://ring-lang.github.io/)
[![][platform]](#)
[![][version]](#)

**An easy-to-use JSON parsing and manipulation library for the Ring programming language**

*Built on [Glaze](https://github.com/stephenberry/glaze) - A high-performance C++ JSON library*

---

</div>

## ✨ Features

-   ⚡ **High Performance**: Built on the blazing-fast Glaze C++ library
-   🌍 **Cross-Platform**: Windows, Linux, macOS, and FreeBSD
-   📝 **Parse & Generate**: Encode/decode JSON with pretty print support
-   🔧 **Manipulation**: Merge, query, validate, compare JSON documents
-   🔀 **Format Conversion**: CSV, TOML, BEVE, CBOR, NDJSON, Base64
-   📋 **RFC Support**: JSON Pointer, JSON Patch, JSON Merge Patch
-   💬 **JSONC**: Parse JSON with comments

## 📋 Prerequisites

-   **Ring Language**: Ensure you have Ring version 1.25 or higher installed. You can download it from the [official Ring website](https://ring-lang.github.io/download.html).

## 🚀 Installation

### Using RingPM

The recommended way to install Ring SimpleJSON is through the Ring Package Manager (RingPM).

```bash
ringpm install simplejson from ysdragon
```

## 💡 Usage

First, load the library in your Ring script:

```ring
load "simplejson.ring"
```

### Parsing JSON Strings

Convert JSON strings into Ring lists:

```ring
jsonString = '{"name": "Ring", "version": 1.25, "features": ["fast", "simple", "flexible"]}'

data = json_decode(jsonString)

# Access data using Ring list syntax
? data[:name]        # Output: Ring
? data[:version]     # Output: 1.25
? data[:features][1]  # Output: fast
```

### Generating JSON

Convert Ring lists into JSON strings:

```ring
# Create a Ring list structure
myData = [
    ["name", "Ring Language"],
    ["version", 1.25],
    ["features", ["fast", "simple", "flexible"]],
    ["active", 1]
]

# Convert to JSON string
jsonString = json_encode(myData)
? jsonString
# Output: {"active":1,"features":["fast","simple","flexible"],"name":"Ring Language","version":1.25}

# Pretty-print with indentation (optional second parameter)
prettyJson = json_encode(myData, 1)
? prettyJson
```

### Working with Complex Data

```ring
# Nested structure
userProfile = [
    ["user", [
        ["name", "John Doe"],
        ["age", 30],
        ["email", "john@example.com"]
    ]],
    ["preferences", [
        ["theme", "dark"],
        ["notifications", 1]
    ]],
    ["posts", [
        ["title", "Hello World", "content", "First post"],
        ["title", "JSON in Ring", "content", "Using SimpleJSON library"]
    ]]
]

# Encode to JSON
jsonData = json_encode(userProfile, 1)
? jsonData

# Decode back to Ring structure
decodedData = json_decode(jsonData)
? decodedData[:user][:name]  # Output: John Doe
```

## 📚 API Reference

### `json_decode(jsonString)`

Parses a JSON string into a Ring list structure.

-   **Parameters:**
    -   `jsonString` (string): The JSON string to parse
-   **Returns:** A Ring list containing the parsed data

### `json_encode(ringList [, prettyPrint])`

Converts a Ring list structure into a JSON string.

-   **Parameters:**
    -   `ringList`: The Ring list to convert
    -   `prettyPrint` (optional): Pass `1`/`TRUE` to format JSON with indentation, `0`/`FALSE` or omit for compact format
-   **Returns:** A JSON string representation of the list data

### `json_version()`

Returns the version of the underlying Glaze library.

-   **Parameters:** None
-   **Returns:** A string representing the Glaze library version

### `json_valid(jsonString)`

Validates whether a string is valid JSON.

-   **Parameters:**
    -   `jsonString` (string): The string to validate
-   **Returns:** `1` if valid JSON, `0` if invalid

```ring
? json_valid('{"name": "John"}')  # Output: 1
? json_valid('{invalid}')         # Output: 0
```

### `json_type(jsonString)`

Returns the type of the root JSON value.

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
-   **Returns:** One of: `"object"`, `"array"`, `"string"`, `"number"`, `"boolean"`, `"null"`, or `"invalid"`

```ring
? json_type('{"a": 1}')   # Output: object
? json_type('[1, 2, 3]')  # Output: array
? json_type('"hello"')    # Output: string
? json_type('42')         # Output: number
? json_type('true')       # Output: boolean
? json_type('null')       # Output: null
```

### `json_keys(jsonString)`

Returns a list of all keys in a JSON object.

-   **Parameters:**
    -   `jsonString` (string): A JSON object string
-   **Returns:** A Ring list containing all keys

```ring
keys = json_keys('{"name": "John", "age": 30, "city": "NYC"}')
for key in keys
    ? key
next
# Output: age, city, name (order may vary)
```

### `json_has(jsonString, key)`

Checks if a key exists in a JSON object.

-   **Parameters:**
    -   `jsonString` (string): A JSON object string
    -   `key` (string): The key to check for
-   **Returns:** `1` if key exists, `0` otherwise

```ring
json = '{"name": "John", "age": 30}'
? json_has(json, "name")   # Output: 1
? json_has(json, "email")  # Output: 0
```

### `json_merge(jsonObject1, jsonObject2)`

Merges two JSON objects. Keys from the second object overwrite keys from the first.

-   **Parameters:**
    -   `jsonObject1` (string): The base JSON object
    -   `jsonObject2` (string): The JSON object to merge in
-   **Returns:** A merged JSON object string

```ring
obj1 = '{"a": 1, "b": 2}'
obj2 = '{"b": 3, "c": 4}'
? json_merge(obj1, obj2)  # Output: {"a":1,"b":3,"c":4}
```

### `json_pointer(jsonString, pointer)`

Access nested values using JSON Pointer syntax (RFC 6901).

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
    -   `pointer` (string): A JSON Pointer path (e.g., `/user/name` or `/items/0`)
-   **Returns:** The value at the specified path (string, number, or list)

```ring
json = '{"user": {"name": "John", "address": {"city": "NYC"}}, "items": [1, 2, 3]}'
? json_pointer(json, "/user/name")          # Output: John
? json_pointer(json, "/user/address/city")  # Output: NYC
? json_pointer(json, "/items/0")            # Output: 1
```

### `json_minify(jsonString)`

Removes all unnecessary whitespace from a JSON string.

-   **Parameters:**
    -   `jsonString` (string): A JSON string (can be pretty-printed)
-   **Returns:** A compact JSON string with no extra whitespace

```ring
pretty = '{
   "name": "John",
   "age": 30
}'
? json_minify(pretty)  # Output: {"name":"John","age":30}
```

### `json_prettify(jsonString)`

Formats a JSON string with indentation for readability.

-   **Parameters:**
    -   `jsonString` (string): A compact JSON string
-   **Returns:** A pretty-printed JSON string with 3-space indentation

```ring
compact = '{"name":"John","age":30}'
? json_prettify(compact)
# Output:
# {
#    "name": "John",
#    "age": 30
# }
```

### `json_read_file(filePath)`

Reads and parses a JSON file.

-   **Parameters:**
    -   `filePath` (string): Path to the JSON file
-   **Returns:** A Ring list containing the parsed data

```ring
data = json_read_file("config.json")
? data[:setting]
```

### `json_write_file(ringList, filePath [, prettyPrint])`

Writes a Ring list to a JSON file.

-   **Parameters:**
    -   `ringList`: The Ring list to write
    -   `filePath` (string): Path to the output file
    -   `prettyPrint` (optional): Pass `1` for formatted output
-   **Returns:** `1` on success

```ring
config = [["name", "MyApp"], ["version", "1.0"]]
json_write_file(config, "config.json", 1)
```

### `json_to_beve(jsonString)`

Converts a JSON string to BEVE (Binary Efficient Versatile Encoding) format.

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
-   **Returns:** Binary BEVE data as a string

```ring
json = '{"test": 123, "hello": "world"}'
beve = json_to_beve(json)
? len(beve)  # Binary data is typically smaller
```

### `beve_to_json(beveData)`

Converts BEVE binary data back to a JSON string.

-   **Parameters:**
    -   `beveData` (string): BEVE binary data
-   **Returns:** A JSON string

```ring
json = '{"test": 123}'
beve = json_to_beve(json)
restored = beve_to_json(beve)
? restored  # Output: {"hello":"world","test":123}
```

### `json_patch(documentJson, operationsJson)`

Applies RFC 6902 JSON Patch operations to a document.

-   **Parameters:**
    -   `documentJson` (string): The target JSON document
    -   `operationsJson` (string): A JSON array of patch operations
-   **Returns:** The patched JSON document string

Supported operations: `add`, `remove`, `replace`, `move`, `copy`, `test`

```ring
doc = '{"name": "John", "age": 30}'
ops = '[{"op": "replace", "path": "/name", "value": "Jane"}, {"op": "add", "path": "/city", "value": "NYC"}]'
result = json_patch(doc, ops)
? result  # Output: {"age":30,"city":"NYC","name":"Jane"}

# Add to array
doc2 = '{"items": [1, 2, 3]}'
ops2 = '[{"op": "add", "path": "/items/-", "value": 4}]'
? json_patch(doc2, ops2)  # Output: {"items":[1,2,3,4]}
```

### `json_diff(sourceJson, targetJson)`

Generates an RFC 6902 JSON Patch that transforms the source into the target.

-   **Parameters:**
    -   `sourceJson` (string): The original JSON document
    -   `targetJson` (string): The target JSON document
-   **Returns:** A JSON array of patch operations

```ring
source = '{"name": "John", "age": 30}'
target = '{"name": "Jane", "age": 30, "city": "NYC"}'
patch = json_diff(source, target)
? patch  # Output: [{"op":"add","path":"/city","value":"NYC"},{"op":"replace","path":"/name","value":"Jane"}]

# Apply the patch to get back to target
? json_patch(source, patch)  # Same as target
```

### `json_merge_patch(targetJson, patchJson)`

Applies an RFC 7386 JSON Merge Patch to a document. Simpler than JSON Patch but less powerful.

-   **Parameters:**
    -   `targetJson` (string): The target JSON document
    -   `patchJson` (string): The merge patch object (null values remove keys)
-   **Returns:** The patched JSON document string

```ring
target = '{"a": "b", "c": {"d": "e", "f": "g"}}'
patch = '{"a": "z", "c": {"f": null}}'
result = json_merge_patch(target, patch)
? result  # Output: {"a":"z","c":{"d":"e"}}  (f was removed by null)

# Simple update
doc = '{"name": "John"}'
update = '{"age": 30, "city": "NYC"}'
? json_merge_patch(doc, update)  # Output: {"age":30,"city":"NYC","name":"John"}
```

### `json_merge_diff(sourceJson, targetJson)`

Generates an RFC 7386 Merge Patch that transforms source into target.

-   **Parameters:**
    -   `sourceJson` (string): The original JSON document
    -   `targetJson` (string): The target JSON document
-   **Returns:** A merge patch object

```ring
source = '{"a": "b", "c": "d"}'
target = '{"a": "b", "c": "e", "f": "g"}'
patch = json_merge_diff(source, target)
? patch  # Output: {"c":"e","f":"g"}
```

### `json_to_cbor(jsonString)`

Converts a JSON string to CBOR (Concise Binary Object Representation) format.

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
-   **Returns:** Binary CBOR data as a string

```ring
json = '{"test": 123, "hello": "world"}'
cbor = json_to_cbor(json)
? len(cbor)  # Binary data length
```

### `cbor_to_json(cborData)`

Converts CBOR binary data back to a JSON string.

-   **Parameters:**
    -   `cborData` (string): CBOR binary data
-   **Returns:** A JSON string

```ring
json = '{"test": 123, "array": [1, 2, 3]}'
cbor = json_to_cbor(json)
restored = cbor_to_json(cbor)
? restored  # Output: {"array":[1,2,3],"test":123}
```

### `json_ndjson_encode(ringList)`

Encodes a Ring list of data structures as NDJSON (Newline Delimited JSON / JSON Lines).

-   **Parameters:**
    -   `ringList`: A Ring list where each item is a data structure to encode
-   **Returns:** An NDJSON string (one JSON object per line)

```ring
records = [
    [["id", 1], ["name", "Alice"]],
    [["id", 2], ["name", "Bob"]],
    [["id", 3], ["name", "Charlie"]]
]
ndjson = json_ndjson_encode(records)
? ndjson
# Output:
# {"id":1,"name":"Alice"}
# {"id":2,"name":"Bob"}
# {"id":3,"name":"Charlie"}
```

### `json_ndjson_decode(ndjsonString)`

Decodes an NDJSON string into a Ring list of data structures.

-   **Parameters:**
    -   `ndjsonString` (string): An NDJSON string (one JSON object per line)
-   **Returns:** A Ring list where each item is a decoded JSON object

```ring
ndjson = '{"a": 1}
{"b": 2}
{"c": 3}'
records = json_ndjson_decode(ndjson)
? len(records)  # Output: 3
for record in records
    ? json_encode(record)
next
# Output:
# {"a":1}
# {"b":2}
# {"c":3}
```

### `json_equal(jsonString1, jsonString2)`

Compares two JSON documents for semantic equality (ignoring key order).

-   **Parameters:**
    -   `jsonString1` (string): First JSON document
    -   `jsonString2` (string): Second JSON document
-   **Returns:** `1` if equal, `0` if not equal

```ring
? json_equal('{"a":1,"b":2}', '{"b":2,"a":1}')  # Output: 1 (same content)
? json_equal('{"a":1}', '{"a":2}')              # Output: 0 (different values)
? json_equal('[1,2,3]', '[1,2,3]')              # Output: 1 (same array)
```

### `json_size(jsonString)`

Returns the number of elements in a JSON object or array.

-   **Parameters:**
    -   `jsonString` (string): A JSON object or array
-   **Returns:** Number of keys (for objects) or elements (for arrays)

```ring
? json_size('{"a":1,"b":2,"c":3}')  # Output: 3
? json_size('[1,2,3,4,5]')          # Output: 5
? json_size('{}')                    # Output: 0
```

### `json_depth(jsonString)`

Returns the maximum nesting depth of a JSON document.

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
-   **Returns:** The maximum nesting depth (0 for scalars)

```ring
? json_depth('{"a":1}')              # Output: 1
? json_depth('{"a":{"b":1}}')        # Output: 2
? json_depth('{"a":{"b":{"c":1}}}')  # Output: 3
```

### `json_sort_keys(jsonString)`

Sorts all object keys alphabetically (recursively).

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
-   **Returns:** JSON string with sorted keys

```ring
? json_sort_keys('{"z":1,"a":2,"m":3}')  # Output: {"a":2,"m":3,"z":1}
```

### `json_query(jsonString, path)`

Queries a JSON document using a path (similar to JSON Pointer).

-   **Parameters:**
    -   `jsonString` (string): A valid JSON string
    -   `path` (string): Path to query (e.g., `/users/0/name`)
-   **Returns:** JSON string of the value at path, or `"null"` if not found

```ring
json = '{"users":[{"name":"Alice"},{"name":"Bob"}]}'
? json_query(json, "/users/0/name")  # Output: "Alice"
? json_query(json, "/missing")       # Output: null
```

### `json_strip_comments(jsoncString)`

Parses JSON with comments (JSONC) and returns standard JSON.

-   **Parameters:**
    -   `jsoncString` (string): JSON string with `//` or `/* */` comments
-   **Returns:** Standard JSON string with comments removed

```ring
jsonc = '{
    // This is a comment
    "name": "test",
    /* Multi-line
       comment */
    "value": 123
}'
? json_strip_comments(jsonc)  # Output: {"name":"test","value":123}
```

### `base64_encode(data)`

Encodes a string to Base64.

-   **Parameters:**
    -   `data` (string): The data to encode
-   **Returns:** Base64 encoded string

```ring
? base64_encode("Hello, World!")  # Output: SGVsbG8sIFdvcmxkIQ==
```

### `base64_decode(encodedString)`

Decodes a Base64 string.

-   **Parameters:**
    -   `encodedString` (string): Base64 encoded string
-   **Returns:** Decoded data

```ring
? base64_decode("SGVsbG8sIFdvcmxkIQ==")  # Output: Hello, World!
```

### `csv_to_json(csvString)`

Converts a CSV string to a JSON array of objects.

-   **Parameters:**
    -   `csvString` (string): CSV data with header row
-   **Returns:** JSON array where each row is an object

```ring
csv = "name,age,city
Alice,30,NYC
Bob,25,LA"
? csv_to_json(csv)
# Output: [{"age":30,"city":"NYC","name":"Alice"},{"age":25,"city":"LA","name":"Bob"}]
```

### `json_to_csv(jsonString)`

Converts a JSON array of objects to CSV format.

-   **Parameters:**
    -   `jsonString` (string): JSON array of objects
-   **Returns:** CSV string with header row

```ring
json = '[{"name":"Alice","age":30},{"name":"Bob","age":25}]'
? json_to_csv(json)
# Output:
# age,name
# 30,Alice
# 25,Bob
```

### `toml_to_json(tomlString)`

Converts a TOML string to JSON.

-   **Parameters:**
    -   `tomlString` (string): TOML formatted data
-   **Returns:** JSON string

```ring
toml = 'title = "Example"
[database]
host = "localhost"
port = 5432'
? toml_to_json(toml)
# Output: {"database":{"host":"localhost","port":5432},"title":"Example"}
```

### `json_to_toml(jsonString)`

Converts a JSON string to TOML format.

-   **Parameters:**
    -   `jsonString` (string): A valid JSON object
-   **Returns:** TOML formatted string

```ring
json = '{"name":"MyApp","port":8080}'
? json_to_toml(json)
# Output:
# name = "MyApp"
# port = 8080
```

## 🛠️ Development

If you wish to contribute to the development of Ring SimpleJSON or build it from the source, follow these steps.

### Prerequisites

-   **CMake**: Version 3.16 or higher.
-   **C++23 Compiler**: A C++ compiler with C++23 support (GCC 13+, Clang 18+, MSVC 2022).
-   **Ring Source Code**: You will need to have the Ring language source code available on your machine.

### Build Steps

1.  **Clone the Repository:**
    ```sh
    git clone https://github.com/ysdragon/simplejson.git --recursive
    ```
    > **Note**: If you installed the library via RingPM, you can skip this step.

2.  **Set the `RING` Environment Variable:**
    This variable must point to the root directory of the Ring language source code.

    -   **Windows (Command Prompt):**
        ```cmd
        set RING=X:\path\to\ring
        ```
    -   **Windows (PowerShell):**
        ```powershell
        $env:RING = "X:\path\to\ring"
        ```
    -   **Unix-like Systems (Linux, macOS or FreeBSD):**
        ```bash
        export RING=/path/to/ring
        ```

3.  **Configure with CMake:**
    Create a build directory and run CMake from within it.
    ```sh
    mkdir build
    cd build
    cmake ..
    ```

4.  **Build the Project:**
    Compile the source code using the build toolchain configured by CMake.
    ```sh
    cmake --build .
    ```

    The compiled library will be available in the `lib/<os>/<arch>` directory.

## 🤝 Contributing

Contributions are always welcome! If you have suggestions for improvements or have identified a bug, please feel free to open an issue or submit a pull request.

## 📄 License

This project is licensed under the MIT License. See the [`LICENSE`](LICENSE) file for more details.