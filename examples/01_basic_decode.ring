load "simplejson.ring"

jsonString = '{"name": "Ring", "version": ' + version() + ', "features": ["fast", "simple", "flexible"]}'

data = json_decode(jsonString)

# Access data using Ring list syntax
? data[:name]        # Output: Ring
? data[:version]     # Output: Ring version
? data[:features][3]  # Output: flexible

? nl + "Done!"