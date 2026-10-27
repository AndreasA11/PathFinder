from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(5, 5, resolution=1.0, ox=0.0, oy=0.0, data=[
    0,   0,   0,   0,   0,
    0, 100, 100, 100,   0,
    0, 100,   0, 100,   0,
    0, 100,   0,   0,   0,
    0,   0,   0,   0,   0
])

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(2.5, 2.5),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"A* planning failed: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == 9, f"Expected optimal shortest path of 9 steps, got {len(poses)}: {reply}"
