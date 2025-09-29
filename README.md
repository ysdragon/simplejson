# Ring SimpleJSON

[license]: https://img.shields.io/github/license/ysdragon/simplejson?style=for-the-badge&logo=opensourcehardware&label=License&logoColor=C0CAF5&labelColor=414868&color=8c73cc
[![][license]](https://github.com/ysdragon/simplejson/blob/master/LICENSE)

An easy-to-use JSON parsing and manipulation library for the Ring programming language. This library is built as a C extension that wraps the high-performance [`Jansson`](https://github.com/akheron/jansson) C JSON library.

## ✨ Features

-   **Parse JSON Strings**: Convert JSON strings into Ring data structures (lists)
-   **Generate JSON**: Convert Ring lists into formatted JSON strings
-   **High Performance**: Built on the fast Jansson C library for optimal speed
-   **Cross-Platform**: Works seamlessly across Windows, Linux, macOS, and FreeBSD
-   **Error Handling**: Comprehensive error reporting for malformed JSON

## 📋 Prerequisites

-   **Ring Language**: Ensure you have Ring version 1.23 or higher installed. You can download it from the [official Ring website](https://ring-lang.github.io/download.html).

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
jsonString = '{"name": "Ring", "version": 1.23, "features": ["fast", "simple", "flexible"]}'

data = json_decode(jsonString)

# Access data using Ring list syntax
? data[:name]        # Output: Ring
? data[:version]     # Output: 1.23
? data[:features][1]  # Output: fast
```

### Generating JSON

Convert Ring lists into JSON strings:

```ring
# Create a Ring list structure
myData = [
    ["name", "Ring Language"],
    ["version", 1.23],
    ["features", ["fast", "simple", "flexible"]],
    ["active", 1]  # true in JSON
]

# Convert to JSON string
jsonString = json_encode(myData)
? jsonString
# Output: {"name": "Ring Language", "version": 1.23, "features": ["fast", "simple", "flexible"], "active": true}

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
        ["notifications", 1]  # true
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

Returns the version of the underlying Jansson library.

-   **Parameters:** None
-   **Returns:** A string representing the Jansson library version

## 🛠️ Development

If you wish to contribute to the development of Ring SimpleJSON or build it from the source, follow these steps.

### Prerequisites

-   **CMake**: Version 3.16 or higher.
-   **C Compiler**: A C compiler compatible with your platform (e.g., GCC, Clang, MSVC).
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