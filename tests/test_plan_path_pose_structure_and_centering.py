from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 2, resolution=1.0, ox=2.0, oy=3.0, data=[0]*6)

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(2.5, 3.5),
        "goal": make_pose_stamped(4.5, 4.5),
        "tolerance": 0.0
    })

assert reply.get("result") is True, f"Planning failed: {reply}"
plan = reply.get("values", {}).get("plan", {})
assert plan.get("header", {}).get("frame_id") == "map", f"Plan header frame_id must be 'map': {plan}"
poses = plan.get("poses", [])
for idx, p in enumerate(poses):
    assert p.get("header", {}).get("frame_id") == "map", f"Pose {idx} header frame_id != 'map'"
    pos = p["pose"]["position"]
    orient = p["pose"]["orientation"]
    assert pos["z"] == 0.0, f"Pose {idx} position.z must be 0.0, got {pos['z']}"
    assert orient == {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}, f"Pose {idx} orientation must be identity quaternion, got {orient}"
