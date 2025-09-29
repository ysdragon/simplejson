load "simplejson.ring"

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

? nl + "Done!"