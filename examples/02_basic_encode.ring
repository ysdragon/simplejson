load "simplejson.ring"

# Create a Ring list structure
myData = [
    ["name", "Ring Language"],
    ["version", 1.23],
    ["features", ["fast", "simple", "flexible"]],
    ["active", json_true()]  # JSON boolean true (not number 1)
]

# Convert to JSON string
jsonString = json_encode(myData)
? jsonString
# Output: {"name":"Ring Language","version":1.23,"features":["fast","simple","flexible"],"active":true}

# Pretty-print with indentation (optional second parameter)
prettyJson = json_encode(myData, 1)
? prettyJson

? nl + "Done!"