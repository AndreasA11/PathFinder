from rosbridge_client import Rosbridge

with Rosbridge() as ros:
    r1 = ros.call_service("/heapify", {"values": ["not_a_number", 1, 2]})
    r2 = ros.call_service("/heap_sort", {"numbers": "not_an_array"})
    r3 = ros.call_service("/heap_sort", {"invalid_key": [1, 2, 3]})

assert r1.get("result") is False, f"/heapify must return result: false for strings: {r1}"
assert r2.get("result") is False, f"/heap_sort must return result: false for non-array: {r2}"
assert r3.get("result") is False, f"/heap_sort must return result: false for missing key: {r3}"
