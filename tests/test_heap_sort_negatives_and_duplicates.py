from rosbridge_client import Rosbridge

numbers = [-4.5, 2.0, -10.0, 0.0, 2.0, -4.5, 100.5]
expected = sorted(numbers)

with Rosbridge() as ros:
    reply = ros.call_service("/heap_sort", {"numbers": numbers})

assert reply.get("result") is True, f"/heap_sort failed on duplicates/negatives: {reply}"
sorted_nums = reply.get("values", {}).get("sorted", [])
assert sorted_nums == expected, f"Expected sorted {expected} but got {sorted_nums}"
