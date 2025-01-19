

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

6) 
	cats->2345 open in browser, webserv immediately quits...


**CHECK**

2) 
	when adding "return 301 /new-route; #redirects request from old route to new-route with a 301 status" to test.conf server 9999, images always has 404

3) 
	why on browser post goes to uploads dir and curl in serv_stuff dir

4) 
	test cgi post + try get cgi requests in browser working

5) 
	test chunked

**CGI**

timeout notes:

// cgiResponse()
{
	// variables
	time_t start_time, current_time;
	int timeout = 3;
	time(&start_time);

	// in cgi parent
	while (1)
	{
		time(&current_time);
		if ((current_time - start_time) >= timeout)
		{
			// http response timeout
			std::cout << "timeout occurred\n";
			kill(_cgi.pid, SIGKILL);
			waitpid(_cgi.pid, NULL, WNOHANG);
			break ;
		}
		sleep (1);
	}
}

