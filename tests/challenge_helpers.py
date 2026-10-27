import math
import time
from typing import Any, List, Tuple
from rosbridge_client import Rosbridge

def make_grid(width: int, height: int, resolution: float = 1.0, ox: float = 0.0, oy: float = 0.0, data: List[int] = None) -> dict:
    if data is None:
        data = [0] * (width * height)
    return {
        "header": {"frame_id": "map"},
        "info": {
            "resolution": float(resolution),
            "width": int(width),
            "height": int(height),
            "origin": {
                "position": {"x": float(ox), "y": float(oy), "z": 0.0},
                "orientation": {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}
            }
        },
        "data": [int(x) for x in data]
    }

def publish_map(ros: Rosbridge, grid: dict, wait: float = 0.1) -> None:
    ros.advertise("/map", "nav_msgs/OccupancyGrid")
    ros.publish("/map", grid)
    time.sleep(wait)

def make_pose_stamped(x: float, y: float) -> dict:
    return {
        "header": {"frame_id": "map"},
        "pose": {
            "position": {"x": float(x), "y": float(y), "z": 0.0},
            "orientation": {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}
        }
    }

def cell_center(cx: int, cy: int, resolution: float = 1.0, ox: float = 0.0, oy: float = 0.0) -> Tuple[float, float]:
    return (ox + (cx + 0.5) * resolution, oy + (cy + 0.5) * resolution)

def is_min_heap(values: List[float]) -> bool:
    n = len(values)
    for i in range(n):
        left = 2 * i + 1
        right = 2 * i + 2
        if left < n and values[i] > values[left]:
            return False
        if right < n and values[i] > values[right]:
            return False
    return True
