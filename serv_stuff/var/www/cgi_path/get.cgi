#!/usr/bin/env python3

import sys
sys.tracebacklimit = 0

try:
	while (1):
		a = 1

	print("Content-Type: text/html\n")

	print("<html>")
	print("<head>")
	print("<title>Simple CGI Script</title>")
	print("</head>")
	print("<body>")
	print("<h1>Hello, World!</h1>")
	print("<p>This is a simple CGI script written in Python.</p>")
	print("</body>")
	print("</html>")

except KeyboardInterrupt:
	sys.exit(0)
