from rosbridge_client import Rosbridge
from challenge_helpers import is_min_heap

numbers = [15, 3, 2, 10, 8, 4, 1, 9, 7, 6]
with Rosbridge() as ros:
    reply = ros.call_service("/heapify", {"values": numbers})

assert reply.get("result") is True, f"/heapify returned false: {reply}"
heap = reply.get("values", {}).get("heap", [])
assert len(heap) == len(numbers), f"Heap length {len(heap)} != expected {len(numbers)}: {reply}"
assert is_min_heap(heap), f"Array does not satisfy min-heap invariant: {heap}"
assert sorted(heap) == sorted(numbers), f"Heap elements do not match input multiset: {heap} vs {numbers}"
