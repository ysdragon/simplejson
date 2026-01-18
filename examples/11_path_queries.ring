load "simplejson.ring"

? "=== JSON Path Queries ==="
? ""

json = '{
    "store": {
        "name": "My Store",
        "products": [
            {"id": 1, "name": "Apple", "price": 1.50},
            {"id": 2, "name": "Banana", "price": 0.75},
            {"id": 3, "name": "Orange", "price": 2.00}
        ],
        "location": {
            "city": "NYC",
            "zip": "10001"
        }
    }
}'

? "--- json_pointer() (RFC 6901) ---"
? "Store name: " + json_pointer(json, "/store/name")
? "First product: " + json_encode(json_pointer(json, "/store/products/0"))
? "Second product name: " + json_pointer(json, "/store/products/1/name")
? "City: " + json_pointer(json, "/store/location/city")
? ""

? "--- json_query() ---"
? "Query /store/name: " + json_query(json, "/store/name")
? "Query /store/products/0/price: " + json_query(json, "/store/products/0/price")
? "Query /store/products/2/name: " + json_query(json, "/store/products/2/name")
? "Query /nonexistent: " + json_query(json, "/nonexistent")
? ""

? "Done!"
