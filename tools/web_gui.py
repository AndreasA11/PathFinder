#!/usr/bin/env python3
"""Project 1 Web Frontend Bridge.

Serves the visualizer web UI at http://127.0.0.1:8080 and bridges HTTP API requests
from the browser to the Project 1 TCP/JSON rosbridge gateway at 127.0.0.1:9095.
"""

from __future__ import annotations

import argparse
import json
import socket
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path
from typing import Any

DEFAULT_WEB_PORT = 8080
DEFAULT_GATEWAY_HOST = "127.0.0.1"
DEFAULT_GATEWAY_PORT = 9095
WEB_ROOT = Path(__file__).resolve().parent / "web"


class RosbridgeBridgeHandler(BaseHTTPRequestHandler):
    gateway_host: str = DEFAULT_GATEWAY_HOST
    gateway_port: int = DEFAULT_GATEWAY_PORT

    def log_message(self, format: str, *args: Any) -> None:
        # Keep console output clean
        pass

    def do_GET(self) -> None:
        if self.path in ("/", "/index.html"):
            index_path = WEB_ROOT / "index.html"
            if not index_path.exists():
                self.send_error(404, "index.html not found")
                return
            content = index_path.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            return

        if self.path == "/api/status":
            online = self.check_gateway()
            body = json.dumps({"online": online}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        self.send_error(404, "Not Found")

    def do_POST(self) -> None:
        if self.path == "/api/rosbridge":
            length = int(self.headers.get("Content-Length", 0))
            if length == 0:
                self.send_error(400, "Empty payload")
                return

            req_bytes = self.rfile.read(length)
            try:
                msg = json.loads(req_bytes.decode("utf-8"))
            except Exception as e:
                self.send_error(400, f"Invalid JSON: {e}")
                return

            # Relay to TCP gateway at 127.0.0.1:9095
            resp_data = self.relay_to_gateway(msg)
            body = json.dumps(resp_data).encode("utf-8")

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        self.send_error(404, "Not Found")

    def check_gateway(self) -> bool:
        try:
            with socket.create_connection((self.gateway_host, self.gateway_port), timeout=0.5):
                return True
        except (OSError, socket.timeout):
            return False

    def relay_to_gateway(self, msg: dict[str, Any]) -> dict[str, Any]:
        try:
            with socket.create_connection((self.gateway_host, self.gateway_port), timeout=3.0) as sock:
                line = json.dumps(msg, separators=(",", ":")).encode("utf-8") + b"\n"
                sock.sendall(line)

                # If calling service, read the service_response line
                if msg.get("op") == "call_service":
                    buf = b""
                    while b"\n" not in buf:
                        chunk = sock.recv(4096)
                        if not chunk:
                            break
                        buf += chunk
                    line_str = buf.split(b"\n")[0].decode("utf-8")
                    return json.loads(line_str)
                else:
                    return {"result": True, "op": "ack"}
        except Exception as e:
            return {"result": False, "error": f"Gateway communication error: {e}"}


def run_server(web_port: int, gw_host: str, gw_port: int) -> None:
    RosbridgeBridgeHandler.gateway_host = gw_host
    RosbridgeBridgeHandler.gateway_port = gw_port
    server = HTTPServer(("127.0.0.1", web_port), RosbridgeBridgeHandler)
    print(f"============================================================")
    print(f"  Project 1 Web Visualizer & Frontend Bridge")
    print(f"  UI: http://127.0.0.1:{web_port}")
    print(f"  Target Gateway: {gw_host}:{gw_port}")
    print(f"============================================================")
    print(f"Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping frontend server.")
        server.server_close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Serve the Project 1 web visualizer.")
    parser.add_argument("--port", default=DEFAULT_WEB_PORT, type=int, help="Web server port (default: 8080)")
    parser.add_argument("--gateway-host", default=DEFAULT_GATEWAY_HOST, help="rosbridge host (default: 127.0.0.1)")
    parser.add_argument("--gateway-port", default=DEFAULT_GATEWAY_PORT, type=int, help="rosbridge port (default: 9095)")
    args = parser.parse_args()

    run_server(args.port, args.gateway_host, args.gateway_port)
    return 0


if __name__ == "__main__":
    sys.exit(main())
