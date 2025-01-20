#!/usr/bin/env python3

# Import the CGI module
import sys

def main():
    # Read input from STDIN
    input_text = sys.stdin.read()

    # Capitalize all letters
    capitalized_text = input_text.upper()

    # Send HTTP headers

    print(capitalized_text)



if __name__ == "__main__":
    main()
