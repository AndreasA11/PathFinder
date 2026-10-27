from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=0.0, oy=0.0, data=[
    100, 0, 0,
    0,   0, 0,
    0,   0, 100
])

with Rosbridge() as ros:
    publish_map(ros, grid)
    r1 = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(1.5, 1.5),
        "tolerance": 0.0
    })
    r2 = ros.call_service("/plan_path", {
        "start": make_pose_stamped(1.5, 1.5),
        "goal": make_pose_stamped(2.5, 2.5),
        "tolerance": 0.0
    })
    r3 = ros.call_service("/plan_path", {
        "start": make_pose_stamped(-5.0, 1.5),
        "goal": make_pose_stamped(1.5, 1.5),
        "tolerance": 0.0
    })
    r4 = ros.call_service("/plan_path", {
        "start": make_pose_stamped(1.5, 1.5),
        "goal": make_pose_stamped(15.0, 1.5),
        "tolerance": 0.0
    })

assert r1.get("result") is False, f"Blocked start cell (100) must return result: false: {r1}"
assert r2.get("result") is False, f"Blocked goal cell (100) must return result: false: {r2}"
assert r3.get("result") is False, f"Negative out-of-bounds start must return result: false: {r3}"
assert r4.get("result") is False, f"Out-of-bounds goal must return result: false: {r4}"
