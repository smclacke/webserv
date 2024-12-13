#!/usr/bin/env python3

# Import the CGI module
import cgi

# Print the HTTP headers (ensure proper formatting)
print("Content-Type: text/html\n")  # This specifies that the content is HTML
print()  # This blank line separates headers from the body

# Start the HTML output
print("<html>")
print("<head>")
print("<title>Simple CGI Script</title>")
print("</head>")
print("<body>")
print("<h1>Hello, World!</h1>")
print("<p>This is a simple CGI script written in Python.</p>")
print("</body>")
print("</html>")