

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

	-> default server:
	---- File extension doesnt match the requested Accept header
	default 8080 throws this error

	-> CATS + CALCULATORS

	-> generate + GET.cpp check variables + if can compile, not sure what is latest
		might have removed some stuff you need but should still be on the server branch



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


