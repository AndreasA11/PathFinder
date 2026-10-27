from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(4, 4, resolution=1.0, ox=0.0, oy=0.0, data=[0]*16)

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(3.5, 3.5),
        "tolerance": 2.5
    })
    assert reply.get("result") is True, f"Request with tolerance failed: {reply}"
    
    for i in range(1, 4):
        r = ros.call_service("/plan_path", {
            "start": make_pose_stamped(0.5, 0.5),
            "goal": make_pose_stamped(float(i) + 0.5, 0.5),
            "tolerance": 0.0
        })
        assert r.get("result") is True, f"Sequential query {i} failed: {r}"
        assert len(r["values"]["plan"]["poses"]) == i + 1, f"Sequential query {i} pose count mismatch"
