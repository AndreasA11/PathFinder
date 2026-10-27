from rosbridge_client import Rosbridge

with Rosbridge() as ros:
    for test_id in ["test-id-alpha", "test-id-beta", "test-id-gamma"]:
        ros.send({
            "op": "call_service",
            "service": "/heap_sort",
            "id": test_id,
            "args": {"numbers": [3, 1, 2]}
        })
        resp = ros.wait_for(lambda m: m.get("op") == "service_response" and m.get("id") == test_id, timeout=5.0)
        assert resp.get("id") == test_id, f"Response ID {resp.get('id')} does not match request ID {test_id}"
        assert resp.get("result") is True, f"Service failed: {resp}"
