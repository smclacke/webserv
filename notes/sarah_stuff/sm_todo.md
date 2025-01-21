

**TODOTODOTODO**

1) 
	any @todos in files

2) 
	add descriptions/briefs

6) 
	cats->2345 open in browser, webserv immediately quits...

7) 	
	Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit").

2) 
	when adding "return 301 /new-route; #redirects request from old route to new-route with a 301 status" to test.conf server 9999, images always has 404

5) 
	if permission denied for post, error in terminal but nothing in browser

5) 
	test chunked

33333) 
	if cgi script (at least for get) doesnt have X rights, its stuck on loading

4444) 
	sometimes php get doesnt have the content, only "Content-Type: plain/text"
	then on reloading it appears
	

9999)     ---- possibly cant recreate but do some stress testing here
	timeout occurred
	handleWrite: send() to client failed
	handleCgiWrite(): write() to cgiIN[1] failed

	program terminated by signal
	^C
	program terminated by signal
	handleCgiWrite(): write() to cgiIN[1] failed
	waitpid error
	free(): invalid pointer
	[1]    76703 IOT instruction (core dumped)  ./webserv config_files/saar.conf

	->> left while in post.cgi, got time out, then removed while from cgi, reload, "program terminated by signal", tried to reload home page, got the rest of the output, crash...

 --- so far cant fully recreate this, no crash yet, maybe ok to leave, its a really mean thing to do anyway
	with loads of refreshing
