from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=5.0, oy=5.0, data=[0]*9)

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(6.2, 7.8),
        "goal": make_pose_stamped(6.8, 7.1),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"start == goal returned false: {reply}"
poses = reply.get("values", {}).get("plan", {}).get("poses", [])
assert len(poses) == 1, f"start == goal must return exactly 1 pose, got {len(poses)}"
pos = poses[0]["pose"]["position"]
assert abs(pos["x"] - 6.5) < 1e-5 and abs(pos["y"] - 7.5) < 1e-5, f"Pose position {pos} is not the exact cell center (6.5, 7.5)"
