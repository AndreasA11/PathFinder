from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=0.0, oy=0.0, data=[
    0,  50, 0,
    0, 100, 0,
    0,   0, 0
])

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(2.5, 0.5),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Plan failed: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == 7, f"Planner did not detour around occupancy >= 50 cells (must take 7 steps, got {len(poses)})"
for p in poses:
    x = p["pose"]["position"]["x"]
    y = p["pose"]["position"]["y"]
    assert not (0.9 < x < 2.1 and (0.0 <= y <= 2.0)), f"Cell with occupancy >= 50 was traversed: ({x}, {y})"
