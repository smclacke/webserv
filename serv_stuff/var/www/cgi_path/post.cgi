#!/usr/bin/env python3

import sys
sys.tracebacklimit = 0

try:
	while (1):
		a = 1

	# Read input from STDIN
	input_text = sys.stdin.read()

	# Capitalize all letters
	capitalized_text = input_text.upper()

	# Send HTTP headers
	print(capitalized_text)

except KeyboardInterrupt:
	sys.exit(0)
