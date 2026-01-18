load "simplejson.ring"

? "=== JSON Merge Patch Examples ==="
? ""

# Basic merge
doc = '{"name": "John", "age": 30, "city": "Boston"}'
? "Original document:"
? json_prettify(doc)
? ""

# Simple update - change name and add email
patch1 = '{"name": "Jane", "email": "jane@example.com"}'
result1 = json_merge_patch(doc, patch1)
? "After merge patch (update name, add email):"
? json_prettify(result1)
? ""

# Remove a field using null
patch2 = '{"city": null}'
result2 = json_merge_patch(result1, patch2)
? "After merge patch (remove city with null):"
? json_prettify(result2)
? ""

# Nested object merge
doc2 = '{"user": {"name": "John", "settings": {"theme": "light", "notifications": true}}}'
? "Nested document:"
? json_prettify(doc2)
? ""

patch3 = '{"user": {"settings": {"theme": "dark", "language": "en"}}}'
result3 = json_merge_patch(doc2, patch3)
? "After nested merge (change theme, add language):"
? json_prettify(result3)
? ""

# Generate merge diff
? "=== Generating Merge Diffs ==="
source = '{"a": 1, "b": 2, "c": 3}'
target = '{"a": 1, "b": 5, "d": 4}'
? "Source: " + source
? "Target: " + target

mergeDiff = json_merge_diff(source, target)
? "Generated merge patch: " + mergeDiff
? "(Note: 'c' removal is represented as null)"
? ""

# Verify by applying
verified = json_merge_patch(source, mergeDiff)
? "Applying merge patch to source: " + verified
? ""

? "Done!"
