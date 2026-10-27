from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped, cell_center

w, h = 5, 4
res = 1.0
grid = make_grid(w, h, resolution=res, ox=0.0, oy=0.0, data=[0]*(w*h))

with Rosbridge() as ros:
    publish_map(ros, grid)
    sx, sy = cell_center(0, 0, res)
    gx, gy = cell_center(w - 1, h - 1, res)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(sx, sy),
        "goal": make_pose_stamped(gx, gy),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Failed planning to boundary cell (x={w-1}, y={h-1}): {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == (w - 1) + (h - 1) + 1, f"Boundary path length wrong: got {len(poses)}"
last_pos = poses[-1]["pose"]["position"]
assert abs(last_pos["x"] - gx) < 1e-5 and abs(last_pos["y"] - gy) < 1e-5, f"Path did not reach outer boundary cell (x={gx}, y={gy}): {last_pos}"
