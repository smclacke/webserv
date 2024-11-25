

**TODOTODOTODO**


recv vs read?
write vs send? both?




// something old im apparently keeping for now

// the socket addresses are the same for server 8080 and server 9999.. this isnt good
// also, after creating sockets, their addresses shouldnt be 0.0.0.0 but 127.0.0.1 but need this is correct format
void		Webserv::printAllServerSocketEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::cout << "epoll epfd = " << _epoll.getEpfd() << " \n\n";
		t_serverData	server = _epoll.getServer(i);
		std::cout << "server socket fd = " << server._server->getServerSocket()->getSockfd() << " \n";
		std::cout << "client socket fd = " << server._server->getClientSocket()->getSockfd() << " \n\n";
		
		sockaddr_in serverSockAddr = server._server->getServerSocket()->getSockaddr();
		socklen_t	servAddrlen = sizeof(serverSockAddr);
		std::cout << "server socket addr = " << &serverSockAddr << " | len = " << servAddrlen << " \n";
		sockaddr_in clientSockAddr = server._server->getClientSocket()->getSockaddr();
		socklen_t	cliAddrlen = sizeof(clientSockAddr);
		std::cout << "client socket addr = " << &clientSockAddr << " | len = " << cliAddrlen << " \n\n";
	}

	//exit(EXIT_SUCCESS);
}


//static void		printSocketAddress(struct sockaddr_in *addr)
//{
//	char	ipStr[INET_ADDRSTRLEN];
//	inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN);
//	std::cout << "Socket Address = " << ipStr << " : " << ntohs(addr->sin_port) << " \n\n";
//}
