

**TODOTODOTODO**


2) 
	add descriptions/briefs

5) 
	error handling + closing + handling of fds/sockets etc
	!!run through full code to check im not missing a close etc
	can then make decsion about this:
		cleanUp function? necessary, anything left behind after success? (i dont think so)

7) 
	check if i need to do any chunky stuff

8) 	
	serious clean up of handleWrite function + any more from epoll.cpp
		+ get rid of unnecessary print statements

9) 
**restart server if fatal error occurs wooooooooo**





111) 
	remove bigWrite when everything gooci




10) 
	**AT THE END, ADD ERRNO CHECKING BACK IN**
	// READ
	//if (errno == EAGAIN || errno == EWOULDBLOCK)
	//{
	//	std::cout << "No data available right now\n";
	//	client._clientState = clientState::BEGIN;
	//	client._connectionClose = false;
	//	return ;
	//}
	// WRITE
	//if (errno == EAGAIN || errno == EWOULDBLOCK)
	//{		
	//	client._clientState = clientState::BEGIN;
	//	client._connectionClose = false;
	//	return ;
	//}
