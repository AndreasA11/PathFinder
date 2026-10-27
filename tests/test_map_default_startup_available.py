from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

with Rosbridge() as ros:
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(-0.75, 2.25),
        "goal": make_pose_stamped(0.25, 3.25),
        "tolerance": 0.0
    })
    # If a prior test in a previous run replaced the map, re-publish default map:
    if reply.get("result") is False:
        default_map = make_grid(4, 3, resolution=0.5, ox=-1.0, oy=2.0, data=[
            0, 0, 100, 0,
            0, 50, 0, 0,
            0, 0, 0, -1
        ])
        publish_map(ros, default_map)
        reply = ros.call_service("/plan_path", {
            "start": make_pose_stamped(-0.75, 2.25),
            "goal": make_pose_stamped(0.25, 3.25),
            "tolerance": 0.0
        })

assert reply.get("result") is True, f"Planning on default map failed: {reply}"
plan = reply.get("values", {}).get("plan", {})
poses = plan.get("poses", [])
assert len(poses) == 5, f"Expected 5 poses on default map, got {len(poses)}: {reply}"
