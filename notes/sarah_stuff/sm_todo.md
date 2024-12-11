

**TODOTODOTODO**

1) 
	any @todos in files

2) 
	add descriptions/briefs


12) 	check-
	It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O
	operations between the client and the server (listen included).
		do i have to add servers before listening?

31) j - friday is option


------------------------------
------------  CGI
------------------------------


32) 
	dont modifyEvent in cgi pipe situation

31) 
	-> isExecutable doesnt seem to like me

30) 

	CURRENT OUTPUT: 
	[[[[[[
		➜  webserv git:(CGI) ✗ ./webserv config_files/cats.conf
		Webserv booting up
		Config: config_files/cats.conf

		Server: "server1"
		Port: 2345
		Host: 127.0.0.1 
		Server socket setup successful

		Server: "server2"
		Port: 9999
		Host: 127.0.0.1 
		Server socket setup successful

		Server: "server3"
		Port: 7777
		Host: 127.0.0.1 
		Server socket setup successful

		added fd to epoll IN :D
		added fd to epoll IN :D
		added fd to epoll IN :D
		added fd to epoll IN :D
		epollin
		epollin
		epollout

		======= REQUEST =======
		GET /var/www/cgi_path/test_file.cgi HTTP/1.1
		Host: localhost:2345
		User-Agent: curl/7.81.0
		Accept: */*


		======= END REQ =======
		added fd to epoll OUT :D
		added fd to epoll IN :D
		Response = HTTP/1.1 200 OK
		Content-Length: 0
		Connection: keep-alive


		epollin
	]]]]]]















