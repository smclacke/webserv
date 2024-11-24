

**TODOTODOTODO**


recv vs read?
write vs send? both?


julius - i need to check im not doing weird shit with sockets - socket sockets + epoll sockets + epoll has addresses and shit tooo


1)
			struct sockaddr_in		address;
			address = thisServer._serverAddr;
			std::cout << "check where address stored " << inet_ntoa(address.sin_addr) << "\n";

		this is zero in monitoring probs cause i dont use the addr etc for server in serverData struct

2)
							if (thisServer._events[j].data.fd == thisServer._serverSock)
				{
					_epoll.makeNewConnection(servers[i]->getServerSocket(), thisServer);
					struct sockaddr_in		address;
					address = thisServer._clients[0]._addr;
					std::cout << "check where address stored " << inet_ntoa(address.sin_addr) << "\n";
				}

		this crashes :)

3)

	else 
	{
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(newSock);
		addToEpoll(newSock, _epfd, server._event);
		server.addClient(newSock, clientAddr, addrLen);
**std::cout << "server new client address = " << inet_ntoa(clientAddr.sin_addr) << "\n";**
		server._clientTime[newSock] = std::chrono::steady_clock::now();
		server.setClientState(clientState::PARSING);
	}


		HERE I GET 127.0.0.1



4)
	possibily shit from opening sockets (addres etc) only used in those functions to open sockets, then use those per server to connect client connections - check this





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
