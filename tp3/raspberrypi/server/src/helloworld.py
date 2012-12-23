#!/usr/bin/python3

#see also http://docs.python.org/3.3/library/http.server.html
from http.server import * 





class HelloHTTPRequestHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		self.send_response(200, "Hello World")

def run(server_class=HTTPServer, handler_class=HelloHTTPRequestHandler):
	server_address = ('', 8000)
	httpd = server_class(server_address, handler_class)
	httpd.serve_forever()

run();

