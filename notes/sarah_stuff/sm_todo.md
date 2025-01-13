

**TODOTODOTODO**

1) 
	any @todos in files

2) 
	add descriptions/briefs

3) 	
	sometimes when things dont work, prog exits without error message + no log for removing sockets/connections/clients from epoll... when unable to load page

-------------------

**testing**

2) 	
	cgi timeout checking + test while(1) in the .cgi

4) 
	rapid siege and browser testing somehow triggered CGI and then crashed everything

5) 	
	memory still having issues.. related to images? if i keep clicking images, it goes up, then run siege with c10 and i get like +300

6) 
	cats->2345 open in browser, webserv immediately quits...


**CHECK**

1) 	
	spaces inbetween server blocks were causing different behaviours with images in browser

2) 
	when adding "return 301 /new-route; #redirects request from old route to new-route with a 301 status" to test.conf server 9999, images always has 404



**CGI**

6) 
	cgi - http 0.9

7) 
	_response cgi bool not set anywhere?

8) 
	location cgi ext in default location init = "" 

9) 
	going into "attempt to GET eecutable file..." error cause _request.cgiReq not true

10) 
	stuck on loading if can get past errors

11) 
	no time out
