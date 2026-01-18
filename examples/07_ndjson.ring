load "simplejson.ring"

? "=== NDJSON Examples ==="
? ""

# Create records as Ring data structures
? "=== Encoding NDJSON ==="
users = [
    [["id", 1], ["name", "Alice"], ["role", "admin"]],
    [["id", 2], ["name", "Bob"], ["role", "user"]],
    [["id", 3], ["name", "Charlie"], ["role", "user"]],
    [["id", 4], ["name", "Diana"], ["role", "moderator"]]
]

# Encode to NDJSON
ndjson = json_ndjson_encode(users)
? "Encoded NDJSON:"
? ndjson
? ""

# Decode NDJSON
? "=== Decoding NDJSON ==="
inputNdjson = '{"event": "login", "user": "alice", "timestamp": 1705312800}
{"event": "purchase", "user": "alice", "item": "book", "price": 29.99}
{"event": "logout", "user": "alice", "timestamp": 1705316400}
{"event": "login", "user": "bob", "timestamp": 1705320000}'

? "Input NDJSON:"
? inputNdjson
? ""

records = json_ndjson_decode(inputNdjson)
? "Decoded " + len(records) + " records:"
for i = 1 to len(records)
    ? "  Record " + i + ": " + json_encode(records[i])
next
? ""

# Process records
? "=== Processing Records ==="
? "Login events:"
for record in records
    if record[:event] = "login"
        ? "  - " + record[:user] + " logged in"
    ok
next
? ""

# Simulating a log processing scenario
? "=== Log Processing Example ==="
logs = [
    [["level", "INFO"], ["message", "Server started"], ["port", 8080]],
    [["level", "DEBUG"], ["message", "Connection received"], ["ip", "192.168.1.100"]],
    [["level", "ERROR"], ["message", "Database timeout"], ["retry", 3]],
    [["level", "INFO"], ["message", "Request processed"], ["duration_ms", 45]]
]

logOutput = json_ndjson_encode(logs)
? "Log output (NDJSON format):"
? logOutput
? ""

? "Done!"
