from rosbridge_client import Rosbridge

numbers = [9, 1, 8, 2, 7, 3, 6, 4, 5, 0]
expected = sorted(numbers)

with Rosbridge() as ros:
    reply = ros.call_service("/heap_sort", {"numbers": numbers})

assert reply.get("result") is True, f"/heap_sort returned false: {reply}"
sorted_nums = reply.get("values", {}).get("sorted", [])
assert sorted_nums == expected, f"Expected {expected} but got {sorted_nums}"
