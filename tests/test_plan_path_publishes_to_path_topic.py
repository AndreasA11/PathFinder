from rosbridge_client import Rosbridge
from challenge_helpers import make_grid, publish_map, make_pose_stamped

grid = make_grid(3, 3, resolution=1.0, ox=0.0, oy=0.0, data=[0]*9)

with Rosbridge() as ros:
    publish_map(ros, grid)
    ros.subscribe("/path", "nav_msgs/Path")
    
    reply = ros.call_service("/plan_path", {
        "start": make_pose_stamped(0.5, 0.5),
        "goal": make_pose_stamped(2.5, 2.5),
        "tolerance": 0.0
    })
    assert reply.get("result") is True, f"Plan call failed: {reply}"
    plan_poses = reply["values"]["plan"]["poses"]
    
    path_msg = ros.next_message("/path", timeout=5.0)
    assert path_msg.get("poses") == plan_poses, f"Published /path msg does not match service response plan: {path_msg}"
