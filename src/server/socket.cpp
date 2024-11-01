/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/01 16:01:45 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Socket::Socket() {}


Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10), _connection(0), _reuseaddr(1)
{
	
	if (type == eSocket::Server)
	{
		if (openServerSocket(servInstance) < 0)
			std::cout << "open server socket error\n\n"; // add throw
		else
			std::cout << "\n\nserver socket setup successful\n\n";

	}
	else if (type == eSocket::Client)
	{
		if (openClientSocket(servInstance) < 0)
			std::cout << "open client socket error\n\n"; // add throw
		else
			std::cout << "\n\nclient socket setup successful\n\n";

	}
	//else
		// throw - incorrect contructor parameter
}

// copy constructor disappeared ...

Socket &Socket::operator=(const Socket &socket)
{
	if (this != &socket)
	{
		// clear all attributes || set to 0

		this->_sockfd = socket._sockfd;
		this->_maxConnections = socket._maxConnections;
		//this->_connection = socket._connection; // copy vector of connections
		this->_sockaddr = socket._sockaddr;
		this->_addrlen = socket._addrlen;
	}
	return *this;
}

Socket::~Socket()
{
	// freeaddrinfo
	closeSockets();
	// clear all attributes (e.g. _addrlen.clear())
	// || set back to 0
	
}


/* methods */

void Socket::closeSockets() // or socket for each instance of socket
{
	//close(this->_connections); // close the vector of connections
	if (_connection > 0)
	{
		close(this->_connection);
		this->_connection = 0;
	}
	if (_sockfd > 0)
	{
		close(this->_sockfd);
		this->_sockfd = 0;
	}
}


// A process that waits passively for requests from clients, processes the work specified,
// and returns the result to the client that originated the request.
int	Socket::openServerSocket(const Server &servInstance)
{

	// create listening socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);
	
	// set to non-blocking socket mode
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags == -1 || fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting nonblocking\n", -1);
	}

	// to re-bind without TIME_WAIT issues
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
		return (std::cout << "error setsockopt sock\n", -1);

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockaddr.sin_port = htons(servInstance.getPort());

	// bind
	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr)) < 0)
		return (std::cout << "error binding sock\n", -1);

	// listen for incoming connections, (set up queue for incoming connections)
	if (listen(_sockfd, 10) < 0) // use _maxConnections
		return (std::cout << "error listening for connections\n", -1);

	std::cout << "listening successfully on port - " << servInstance.getPort() << " \n";
	
	while (true)
	{	
		// accept connection
		_addrlen = sizeof(_sockaddr);
		_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addrlen);
		if (_connection < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue ;
			return (std::cout << "error accepting connection\n", -1);
		}

		// set connection to nonblocking
		flags = fcntl(_connection, F_GETFL, 0);
		if (flags == -1 || fcntl(_connection, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			close(_connection);	
			continue ;
		}

		std::cout << "successfully made connection\n";

		// read from the connection
		char	buffer[1000];
		std::string	request;
		ssize_t bytesRead;
		while (bytesRead = read(_connection, buffer, sizeof(buffer) - 1) > 0)
		{
			buffer[bytesRead] = '\0';
			request += buffer;	
		}
		
		if (bytesRead < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				std::cout << "no data available to read\n";
				continue ;
			}
			return (std::cout << "error reading from server\n", -1);
		}
		buffer[99] = '\0';
		std::cout << "read by server: <" << buffer << ">\n";
		
		// send response message to the connection
		std::string	response = "response message from server";
		if (send(_connection, response.c_str(), response.size(), 0) < 0)
			return (std::cout << "error sending from server\n", -1);

		std::cout << "sent from server: <" << response << ">\n";
	}

	return 1;
}


// A process that initiates a service request.
// connect to server sockt instead of bind
int Socket::openClientSocket(const Server &servInstance)
{

	// create client socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = inet_addr(servInstance.getHost().c_str());
	_sockaddr.sin_port = htons(servInstance.getPort());

	// set to non-blocking socket mode
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags == -1 || fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting client socket to nonblocking\n", -1);
	}

	// attempt to connect
	_addrlen = sizeof(_sockaddr);
	if ((connect(_sockfd, (struct sockaddr *) &_sockaddr, _addrlen)) < 0)
	{
		if (errno == EINPROGRESS)
		{
			std::cout << "connection in progress... \n";
			fd_set	writefds;
			FD_ZERO(&writefds);
			FD_SET(_sockfd, &writefds);
			struct timeval	tv;
			tv.tv_sec = 5; // timeout for 5 seconds
			tv.tv_usec = 0;

			int	result = select(_sockfd + 1, nullptr, &writefds, nullptr, &tv);
			if (result <= 0)
			{
				close (_sockfd);
				return (std::cout <<"error or timeout waiting for connection\n", -1);
			}
		}
		else
		{
			close(_sockfd);
			return (std::cout << "error connecting to server from client\n", -1);
		}
	}
	std::cout << "client connected successfully to port - " << servInstance.getPort() << " \n";
	
	// send message to server socket 
	std::string	message = "message from client";
	if (send(_sockfd, message.c_str(), message.size(), 0) < 0)
	{
		close(_sockfd);
		return (std::cout << "error sending from client\n", -1);
	}
	std::cout << "sent from client: <" << message << ">\n";
	
	// read response from server
	char	buffer[100];
	ssize_t bytesRead = read(_sockfd, buffer, sizeof(buffer) - 1);
	if (bytesRead < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return (std::cout << "no data available to read\n", 0);
		else
		{
			close(_sockfd);
			return (std::cout << "error reading from client\n", -1);
		}
	}
	buffer[99] = '\0';
	std::cout << "read by client: <" << buffer << ">\n";

	return 1;
}
