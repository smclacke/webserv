

**TODOTODOTODO**


2) 
	add descriptions/briefs

7) 
	check if i need to do any chunky stuff

8) 	
	clean up readFile bools - think theres like 3 on the go...

9) 
	**restart server if fatal error occurs wooooooooo**

10) 
	portchecking logic needs to change, before it is checked, server sockets are created and bind() is called, and fails

11) 
	default server:
---- File extension doesnt match the requested Accept header
	default 8080 throws this error






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


