load "simplejson.ring"

jsonString = '{"name": "Ring", "version": 1.23, "features": ["fast", "simple", "flexible"]}'

data = json_decode(jsonString)

# Access data using Ring list syntax
? data[:name]        # Output: Ring
? data[:version]     # Output: 1.23
? data[:features][1]  # Output: fast

? nl + "Done!"