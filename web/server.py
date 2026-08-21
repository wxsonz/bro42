#!/usr/bin/env python3
import http.server
import socketserver
import os
import sys
import subprocess
import json

PORT = 4242
DIRECTORY = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.dirname(DIRECTORY)

class BroHandler(http.server.SimpleHTTPRequestHandler):
	def __init__(self, *args, **kwargs):
		super().__init__(*args, directory=DIRECTORY, **kwargs)

	def do_POST(self):
		if self.path == '/api/run':
			# Run bro launcher to re-compile student code and refresh report.json
			bro_launcher = os.path.join(BASE_DIR, "bro")
			ref_dir = os.environ.get("FT_BRO_TARGET", os.environ.get("FT_COMPANION_TARGET", os.path.join(BASE_DIR, "_dev", "reference", "libft42git")))
			
			if os.path.exists(bro_launcher):
				cmd = [bro_launcher, ref_dir, "--json", "--no-web"]
				subprocess.run(cmd)

			report_path = os.path.join(DIRECTORY, "report.json")
			if os.path.exists(report_path):
				with open(report_path, "r") as f:
					data = f.read()
				self.send_response(200)
				self.send_header("Content-type", "application/json")
				self.end_headers()
				self.wfile.write(data.encode('utf-8'))
			else:
				self.send_response(500)
				self.end_headers()
		else:
			self.send_response(404)
			self.end_headers()

def is_port_in_use(port):
	import socket
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		return s.connect_ex(('localhost', port)) == 0

def run_server():
	if is_port_in_use(PORT):
		# Server is already running
		sys.exit(0)
	os.chdir(DIRECTORY)
	socketserver.TCPServer.allow_reuse_address = True
	with socketserver.TCPServer(("", PORT), BroHandler) as httpd:
		print(f"🚀 ft_bro Web Dashboard running at http://localhost:{PORT}")
		try:
			httpd.serve_forever()
		except KeyboardInterrupt:
			pass

if __name__ == "__main__":
	run_server()
