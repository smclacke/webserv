

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