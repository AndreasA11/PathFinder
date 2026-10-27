from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=0.0, oy=0.0, data=[
    0, 100, 0,
    0, 100, 0,
    0, 100, 0
])

with Rosbridge() as ros:
    publish_map(ros, grid)
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(2.5, 2.5),
        "tolerance": 0.0
    })

assert reply.get("result") is False, f"Unreachable goal should return result: false, got: {reply}"
