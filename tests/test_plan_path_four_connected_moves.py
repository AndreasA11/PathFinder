from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

res = 0.5
grid = make_grid(4, 4, resolution=res, ox=0.0, oy=0.0, data=[0]*16)

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.25, 0.25),
        "goal": make_pose_stamped(1.75, 1.75),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Planning failed: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) >= 2, f"Expected multiple poses, got {len(poses)}"

for i in range(len(poses) - 1):
    p1 = poses[i]["pose"]["position"]
    p2 = poses[i+1]["pose"]["position"]
    dx = round(abs(p2["x"] - p1["x"]) / res, 3)
    dy = round(abs(p2["y"] - p1["y"]) / res, 3)
    assert (dx == 1.0 and dy == 0.0) or (dx == 0.0 and dy == 1.0), f"Illegal move between step {i} and {i+1}: dx={dx}, dy={dy} (must be cardinal 4-connected)"
