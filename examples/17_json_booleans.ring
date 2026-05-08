load "simplejson.ring"

# ============================================================================
# JSON Booleans
# Ring has no native boolean type (true/false are numbers 1/0).
# Use json_true() and json_false() to create proper JSON booleans.
# ============================================================================

# --- Creating JSON booleans ---
myData = [
    ["active", json_true()],
    ["disabled", json_false()],
    ["count", 1]  # Regular number, NOT a boolean
]
? "Encode:"
? json_encode(myData)
# {"active":true,"count":1,"disabled":false}

# --- Decoding JSON booleans ---
? nl + "Decode:"
data = json_decode('{"enabled": true, "muted": false, "score": 1}')
? json_is_true(data[:enabled])   # 1
? json_is_false(data[:muted])    # 1
? json_tobool(data[:enabled])    # 1
? json_tobool(data[:muted])      # 0

# --- Round-trip: booleans preserved ---
? nl + "Round-trip:"
original = '{"active": true, "disabled": false, "count": 1}'
decoded = json_decode(original)
restored = json_encode(decoded)
? restored
# {"active":true,"count":1,"disabled":false}

? nl + "Done!"
