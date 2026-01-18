load "simplejson.ring"

? "=== JSON Patch Examples ==="
? ""

# Basic document
doc = '{"name": "John", "age": 30, "city": "Boston"}'
? "Original document:"
? json_prettify(doc)
? ""

# Replace a value
ops1 = '[{"op": "replace", "path": "/name", "value": "Jane"}]'
result1 = json_patch(doc, ops1)
? "After replacing name:"
? json_prettify(result1)
? ""

# Add a new field
ops2 = '[{"op": "add", "path": "/email", "value": "jane@example.com"}]'
result2 = json_patch(result1, ops2)
? "After adding email:"
? json_prettify(result2)
? ""

# Remove a field
ops3 = '[{"op": "remove", "path": "/city"}]'
result3 = json_patch(result2, ops3)
? "After removing city:"
? json_prettify(result3)
? ""

# Multiple operations at once
doc2 = '{"items": ["apple", "banana"], "count": 2}'
ops4 = '[
    {"op": "add", "path": "/items/-", "value": "cherry"},
    {"op": "replace", "path": "/count", "value": 3},
    {"op": "add", "path": "/updated", "value": true}
]'
result4 = json_patch(doc2, ops4)
? "Multiple operations on array:"
? json_prettify(result4)
? ""

# Generate a diff between two documents
? "=== Generating Diffs ==="
source = '{"name": "John", "age": 30}'
target = '{"name": "Jane", "age": 31, "city": "NYC"}'
? "Source: " + source
? "Target: " + target
diff = json_diff(source, target)
? "Generated patch:"
? json_prettify(diff)
? ""

# Apply the generated patch to verify
verified = json_patch(source, diff)
? "Applying patch to source gives: " + verified
? ""

? "Done!"
