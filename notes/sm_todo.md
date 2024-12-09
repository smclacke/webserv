

**TODOTODOTODO**


2) 
	add descriptions/briefs

7) 
	check if i need to do any chunky stuff

9) 
	**restart server if fatal error occurs wooooooooo**

12) 	check-
	It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O
	operations between the client and the server (listen included).
		do i have to add servers before listening?

13) 
	test big files + large images


10) JULIUS UPDATES: 

	-> portchecking logic needed to change, bind() throws so it's checked already
		in server socket creation

11) 
	running siege causes "Fialed to read request line" error + EPOLLHUP + EPOLLERR




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


