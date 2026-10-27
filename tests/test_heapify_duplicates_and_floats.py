from rosbridge_client import Rosbridge
from challenge_helpers import is_min_heap

numbers = [3.5, -2.1, 0.0, 3.5, -2.1, -10.0, 5.25]
with Rosbridge() as ros:
    reply = ros.call_service("/heapify", {"values": numbers})

assert reply.get("result") is True, f"/heapify failed with duplicates/floats/negatives: {reply}"
heap = reply.get("values", {}).get("heap", [])
assert len(heap) == len(numbers), f"Heap size mismatch: {reply}"
assert is_min_heap(heap), f"Min-heap property failed with negative/float numbers: {heap}"
assert sorted(heap) == sorted(numbers), f"Multiset not preserved for duplicates/floats: {heap}"
