from rosbridge_client import Rosbridge

with Rosbridge() as ros:
    r_heapify = ros.call_service("/heapify", {"values": []})
    r_sort = ros.call_service("/heap_sort", {"numbers": []})

assert r_heapify.get("result") is True, f"/heapify with empty array failed: {r_heapify}"
assert r_heapify.get("values", {}).get("heap") == [], f"Expected empty heap: {r_heapify}"

assert r_sort.get("result") is True, f"/heap_sort with empty array failed: {r_sort}"
assert r_sort.get("values", {}).get("sorted") == [], f"Expected empty sorted list: {r_sort}"
