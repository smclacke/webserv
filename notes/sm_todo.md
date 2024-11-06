

**TODOTODOTODO**


1) check with julius how we're handling try/catch so i can just do it all now correctly (will main catch any throw?)
-- for multiple, when server has been through requests etc, server sockets will be closed
		and deleted from epoll to make space for the next server

2) stuff in main - figure out with julius (multiple server monitoring etc //server[1]...)

3) close protection + double check other cleaning

4) vector of connections, how what where why



-----------------------------------------------------------------

**WHEREIAT**

- got two client sort of versions in epoll loop, testing extra server stuff

- not totally erroring, curl give HTTP 0.9 not allowed
- browser no message page not found :( locahost refused to connect / not allowed something



// moving out the way
	
	// SERVER ADDRESS AND PORT
	//memset(&_sockaddr, 0, sizeof(_sockaddr));
	//_sockaddr.sin_family = AF_INET;
	//_sockaddr.sin_port = htons(servInstance.getPort());
	//inet_pton(AF_INET, servInstance.getHost().c_str(), &_sockaddr.sin_addr);
	//setSockfd(_sockfd);


	//_host = servInstance.getHost();
	//setHost(_host);
	

	// CONNECT

	///* TESTING OLD CLIENT FUNCTION FROM HERE */
	//// attempt to connect
	//if ((connect(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen)) < 0)
	//{
	//	close(_sockfd);
	//	throw std::runtime_error("Error connecting to server from client\n");
	//}
	//std::cout << "Client connected successfully to port - " << servInstance.getPort() << " \n";

	//std::string message = "GET / HTTP/1.1\r\nHost: " + servInstance.getHost() + "\r\nConnection: close\r\n";
	//if (send(_sockfd, message.c_str(), message.size(),0) < 0)
	//{
	//	close(_sockfd);
	//	throw std::runtime_error("Error sending message from client\n");
	//}
	
	//char	buffer[1000];
	//ssize_t	bytesRead;
	//while ((bytesRead = read(_sockfd, buffer, sizeof(buffer) - 1)) > 0)
	//{
	//	buffer[bytesRead] = '\0';
	//	std::cout << "Received response: " << buffer;
	//}
	//if (bytesRead < 0)
	//	throw std::runtime_error("Error reading response\n");
	//buffer[999] = '\0';
	//std::cout << "Read by client: " << buffer << "\n";




				//else
			//{
			//	/* TESTING OLD CLIENT FUNCTION FROM HERE */
			//	//int		clientSockfd = events[i].data.fd; //events[i].data.fd
			//	//socklen_t	clientAddrlen = client.getAddrlen();
			//	//struct sockaddr_in	clientSockaddr = client.getSockaddr();
				
			//	int clientSockfd = client.getSockfd();
			//	if ((connect(clientSockfd, (struct sockaddr *)&serverSockaddr, serverAddrlen)) < 0)
			//	{
			//		close(clientSockfd);
			//		throw std::runtime_error("Error connecting to server from client\n");
			//	}
			//	std::cout << "Client connected successfully to port\n";

			//	std::string message = "GET / HTTP/1.1\r\nHost: " + client.getHost() + "\r\nConnection: close\r\n";
			//	if (send(clientSockfd, message.c_str(), message.size(),0) < 0)
			//	{
			//		close(clientSockfd);
			//		throw std::runtime_error("Error sending message from client\n");
			//	}
				
			//	char	buffer[1000];
			//	ssize_t	bytesRead;
			//	while ((bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1)) > 0)
			//	{
			//		buffer[bytesRead] = '\0';
			//		std::cout << "Received response: " << buffer;
			//	}
			//	if (bytesRead < 0)
			//		throw std::runtime_error("Error reading response\n");
			//	buffer[999] = '\0';
			//	std::cout << "Read by client: " << buffer << "\n";
			//	closeDelete(clientSockfd, _epfd);
			//	std::cout << "\nClosed client socket and deleted from Epoll\n";
				
			//}
			
			
			/* KEEP GETTING ERROR READING FROM CLIENT THROW WITH BELOW CODE */
			//else
			//{
			//	// handle incoming data from client connection
			//	int			clientSockfd = events[i].data.fd;
			//	char		buffer[1024];
			//	ssize_t		bytesRead; // = 0;
			//	std::string	request;

			//	//while ((bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1)) > 0)
			//	//{
			//	//	buffer[bytesRead] = '\0';
			//	//	request.append(buffer);
			//	//}
			//	bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1);
			//	if (bytesRead > 0)
			//	{
			//		buffer[bytesRead] = '\0';
			//		std::cout << "Received request: " << request << "\n";
			//		std::string response = generateHttpResponse("HELLO WORLLLLLD");
			//		send(clientSockfd, response.c_str(), response.size(), 0);
			//	}
			//	//if (!request.empty())
			//		//std::cout << "Received request: " << request << "\n";
			//	//else
			//	//	std::cout << "Request empty\n";
				
			//	if (bytesRead < 0)
			//	{
			//		closeDelete(clientSockfd, _epfd);
			//		throw std::runtime_error("Error reading from client\n");
			//	}
			//	closeDelete(clientSockfd, _epfd);
			//	std::cout << "\nClosed client socket and deleted from Epoll\n";
			//}
		//}