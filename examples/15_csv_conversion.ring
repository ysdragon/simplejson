load "simplejson.ring"

? "=== CSV Conversion ==="
? ""

? "--- csv_to_json() ---"
csv = "name,age,city,active
Alice,30,NYC,true
Bob,25,LA,false
Charlie,35,Chicago,true"

? "Input CSV:"
? csv
? ""

json = csv_to_json(csv)
? "Output JSON:"
? json_prettify(json)
? ""

? "--- json_to_csv() ---"
jsonData = '[
    {"name": "Alice", "age": 30, "city": "NYC"},
    {"name": "Bob", "age": 25, "city": "LA"},
    {"name": "Charlie", "age": 35, "city": "Chicago"}
]'

? "Input JSON:"
? jsonData
? ""

csvOutput = json_to_csv(jsonData)
? "Output CSV:"
? csvOutput
? ""

? "--- Round-trip ---"
original = "product,price,stock
Apple,1.50,100
Banana,0.75,200
Orange,2.00,150"

? "Original CSV:"
? original
? ""

roundTrip = json_to_csv(csv_to_json(original))
? "After round-trip:"
? roundTrip

? "Done!"
