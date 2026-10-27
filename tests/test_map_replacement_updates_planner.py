from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=0.0, oy=0.0, data=[
    0,   0,   0,
    0, 100,   0,
    0,   0,   0
])

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(2.5, 2.5),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Planning failed after new map replacement: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == 5, f"Detour around center obstacle should take 5 poses, got {len(poses)}"
for p in poses:
    x = p["pose"]["position"]["x"]
    y = p["pose"]["position"]["y"]
    assert not (0.9 < x < 2.1 and 0.9 < y < 2.1), f"Path illegally traversed center obstacle (1,1): ({x},{y})"
