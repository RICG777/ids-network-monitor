#!/usr/bin/env python3
"""Static file server with aggressive no-cache headers. For local dev only.

Usage: python dev-server.py [port]
Default port: 8000.
"""

import http.server
import socketserver
import sys

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8000


class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Force full revalidation on every request so ES module changes land
        # without restarting the browser. Safe since this is dev-only.
        self.send_header("Cache-Control", "no-store, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()


if __name__ == "__main__":
    with socketserver.TCPServer(("", PORT), NoCacheHandler) as httpd:
        print(f"Serving on http://localhost:{PORT}/ (no-cache mode)")
        httpd.serve_forever()
