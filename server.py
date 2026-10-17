import http.server
import socketserver
import json
import csv
import os

PORT = 8080

class FraudAPIHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/api/transactions':
            data = []
            if os.path.exists("audit_trail.csv"):
                with open("audit_trail.csv", "r") as f:
                    reader = csv.DictReader(f)
                    for row in reader:
                        data.append(row)
            recent_data = data[-15:]
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(recent_data).encode('utf-8'))
        else:
            super().do_GET()

print(f"Web Dashboard running on port {PORT}")
with socketserver.TCPServer(("", PORT), FraudAPIHandler) as httpd:
    httpd.serve_forever()
