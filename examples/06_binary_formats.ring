load "simplejson.ring"

? "=== Binary Format Examples ==="
? ""

# Sample JSON data
json = '{"user": "Alice", "scores": [95, 87, 92], "active": true, "metadata": {"created": "2024-01-15"}}'
? "Original JSON:"
? json_prettify(json)
? "JSON size: " + len(json) + " bytes"
? ""

# Convert to BEVE (Binary Efficient Versatile Encoding)
? "=== BEVE Format ==="
beve = json_to_beve(json)
? "BEVE size: " + len(beve) + " bytes"
? "Compression ratio: " + (len(beve) * 100 / len(json)) + "%"

# Convert back
restored_beve = beve_to_json(beve)
? "Restored from BEVE: " + restored_beve
? ""

# Convert to CBOR (Concise Binary Object Representation)
? "=== CBOR Format ==="
cbor = json_to_cbor(json)
? "CBOR size: " + len(cbor) + " bytes"
? "Compression ratio: " + (len(cbor) * 100 / len(json)) + "%"

# Convert back
restored_cbor = cbor_to_json(cbor)
? "Restored from CBOR: " + restored_cbor
? ""

# Larger example to show compression benefits
? "=== Larger Data Example ==="
largeJson = '{"records": ['
for i = 1 to 10
    if i > 1 largeJson += "," ok
    largeJson += '{"id": ' + i + ', "name": "User' + i + '", "email": "user' + i + '@example.com"}'
next
largeJson += ']}'

? "Large JSON size: " + len(largeJson) + " bytes"

largeBeve = json_to_beve(largeJson)
? "Large BEVE size: " + len(largeBeve) + " bytes (" + (len(largeBeve) * 100 / len(largeJson)) + "%)"

largeCbor = json_to_cbor(largeJson)
? "Large CBOR size: " + len(largeCbor) + " bytes (" + (len(largeCbor) * 100 / len(largeJson)) + "%)"
? ""

? "Done!"
