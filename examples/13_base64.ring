load "simplejson.ring"

? "=== Base64 Encoding/Decoding ==="
? ""

? "--- base64_encode() ---"
text = "Hello, World!"
encoded = base64_encode(text)
? "Original: " + text
? "Encoded: " + encoded
? ""

? "--- base64_decode() ---"
decoded = base64_decode(encoded)
? "Decoded: " + decoded
? ""

? "--- Practical example: Binary data in JSON ---"
binaryData = char(0) + char(255) + char(128) + "binary"
encoded64 = base64_encode(binaryData)

jsonWithBinary = json_encode([
    ["filename", "data.bin"],
    ["content", encoded64],
    ["size", len(binaryData)]
])
? "JSON with base64 content:"
? json_prettify(jsonWithBinary)
? ""

? "--- Round-trip verification ---"
data = json_decode(jsonWithBinary)
restored = base64_decode(data[:content])
? "Original length: " + len(binaryData)
? "Restored length: " + len(restored)
? "Match: " + (binaryData = restored)
? ""

? "Done!"
