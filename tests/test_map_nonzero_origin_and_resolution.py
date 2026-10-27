from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped, cell_center

ox, oy = 10.0, -20.0
res = 2.5
grid = make_grid(4, 3, resolution=res, ox=ox, oy=oy, data=[0]*12)

with Rosbridge() as ros:
    publish_map(ros, grid)
    sx, sy = cell_center(0, 0, res, ox, oy)
    gx, gy = cell_center(3, 2, res, ox, oy)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(sx, sy),
        "goal": make_pose_stamped(gx, gy),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Failed planning on nonzero origin/resolution map: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == 6, f"Expected Manhattan path of 6 poses, got {len(poses)}"

first_pos = poses[0]["pose"]["position"]
last_pos = poses[-1]["pose"]["position"]
assert abs(first_pos["x"] - sx) < 1e-5 and abs(first_pos["y"] - sy) < 1e-5, f"Start pose {first_pos} does not match expected cell center ({sx},{sy})"
assert abs(last_pos["x"] - gx) < 1e-5 and abs(last_pos["y"] - gy) < 1e-5, f"Goal pose {last_pos} does not match expected cell center ({gx},{gy})"
