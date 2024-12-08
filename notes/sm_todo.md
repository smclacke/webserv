

**TODOTODOTODO**


2) 
	add descriptions/briefs

7) 
	check if i need to do any chunky stuff

9) 
	**restart server if fatal error occurs wooooooooo**

10) 
	julius update:	portchecking logic needs to change, before it is checked, server sockets are created and bind() is called, and fails

11) 
	default server:
---- File extension doesnt match the requested Accept header
	default 8080 throws this error


11) 
	continuous clicking on buttons:
	http://localhost:9999/calculator/calculator/calculator/images/images/images/images.html

12) 	check-
	It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O
	operations between the client and the server (listen included).
		do i have to add servers before listening?

13) 
	what is up with the image button?

100) 
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


