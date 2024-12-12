

**TODOTODOTODO**

1) 
	any @todos in files

2) 
	add descriptions/briefs


12) 	check-
	It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O
	operations between the client and the server (listen included).
		do i have to add servers before listening?


11111) **NOW**

	- parsUpdates doesn't compile - and now it does... web page buttons dont do anything

	- CGI can't handle basic images anymore in browser + calculator broken + checked server_files + conf + index but can't see any issues

	- isCgi doesnt work? - was an issue, maybe not anymore?

	- always throws error:	
		return setErrorResponse(eHttpStatusCode::Forbidden, "file extension does not match accepted cgi extension");
		
	- sort of changing the index.html... but really weird behaviour:

--------------------------

[garbage in strings]

======= END REQ HEADERS =======
Invalid request line format
Response = HTTP/1.1 400 Bad Request
Content-Length: 262
Connection: keep-alive

Resource not found at path: ./server_files/calcul%EF%BF%BDtor/calculator.htmlInvalid request line formatHTTP method not allowed: User-Agent:Invalid request line formatInvalid request line formatHTTP method not allowed: Accept-Encoding:Invalid request line format

------------------

[calculator url]

http://localhost:2345/calcul%EF%BF%BDtor/calculator.html


-----------------------

[calculator button press]

Resource not found at path: ./server_files/calcul%EF%BF%BDtor/calculator.htmlInvalid request line format

-----------------------

[if large image button pressed]

style> body { font-family: Arial, sans-serif; text-align: center; padding: 50px; } h1 { font-size: 50px; } { font-size: 20px; }
Error 405
p>Method Not Allowed

-----------------------

