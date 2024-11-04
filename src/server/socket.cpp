/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/04 16:42:23 by eugene        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Socket::Socket() {}

Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10), _connection(0), _reuseaddr(1), _flags(0)
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
Socket::Socket(const Socket &copy)
{
	*this = copy;
}

Socket &Socket::operator=(const Socket &socket)
{
	if (this != &socket)
	{
		this->_sockfd = socket._sockfd;
		this->_maxConnections = socket._maxConnections;
		this->_connection = socket._connection;
		//this->_connection = socket._connection; // copy vector of connections
		this->_sockaddr = socket._sockaddr;
		this->_addrlen = socket._addrlen;
		this->_reuseaddr = socket._reuseaddr;
		this->_buffer[BUFFER_SIZE] = socket._buffer[BUFFER_SIZE];
		this->_request = socket._request;
		this->_bytesRead = socket._bytesRead;
		this->_bufferSize = socket._bufferSize;
	}
	return *this;
}

Socket::~Socket()
{
	// freeaddrinfo
	closeSocket();
	// clear all attributes (e.g. _addrlen.clear())
	// || set back to 0
	
}


/* methods */

void Socket::closeSocket() 
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

// will get this function from Julius at some point
static std::string generateHttpResponse(const std::string &message)
{
	size_t	contentLength = message.size();
	std::ostringstream	response;
	response	<< "HTPP/1.1 200 OK\r\n"
				<< "Content-Type: text/plain\r\n"
				<< "Content-Length: " << contentLength << "\r\n"
				<< "Connection : close\r\n"
				<< "\r\n"
				<< message;
	
	return response.str();
}


// A process that waits passively for requests from clients, processes the work specified,
// and returns the result to the client that originated the request.
int	Socket::openServerSocket(const Server &servInstance)
{

	// create listening socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);
	
	// set to non-blocking socket mode
	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting nonblocking\n", -1);
	}

	// to re-bind without TIME_WAIT issues
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
		return (std::cout << "error setsockopt sock\n", -1);

	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockaddr.sin_port = htons(servInstance.getPort());

	// bind
	_addrlen = sizeof(_sockaddr);
	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen) < 0)
	{
		close(_sockfd);
		return (std::cout << "error binding sock\n", -1);
	}

	// listen for incoming connections, (set up queue for incoming connections)
	if (listen(_sockfd, _maxConnections) < 0)
	{
		close(_sockfd);
		return (std::cerr << "error listening for connections\n", -1);
	}

	std::cout << "listening successfully on port - " << servInstance.getPort() << " \n";
	
	// epoll comes in here

	while (true)
	{	
		// accept connection
		_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
		if (_connection < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue ;
			std::cerr << "error accepting connection\n";
			continue ;
		}

		std::cout << "successfully made connection\n";

		// read from the connection
		_bufferSize = sizeof(_buffer);
		while ((_bytesRead = read(_connection, _buffer, _bufferSize - 1)) > 0)
		{
			_buffer[_bytesRead] = '\0';
			_request += _buffer;
			if (_request.find("\r\n\r\n") != std::string::npos)
				break ; // end of HTTP request
		}
		
		if (_bytesRead < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
			std::cerr << "error reading from connection\n";

		if (!_request.empty())
		{
			std::cout << "received request: " << _request << "\n";

			// send HTTP response
			std::string	_response = generateHttpResponse(std::string("get me on your browser"));
			if (send(_connection, _response.c_str(), _response.size(), 0) < 0)
				std::cerr << "error snding response to client\n";
		}
		else
			std::cout << "received empty request\n";

		close(_connection);
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

	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons(servInstance.getPort());
	inet_pton(AF_INET, servInstance.getHost().c_str(), &_sockaddr.sin_addr);

	// set to non-blocking socket mode
	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting client socket to nonblocking\n", -1);
	}

	// attempt to connect
	_addrlen = sizeof(_sockaddr);
	if ((connect(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen)) < 0)
	{
		close(_sockfd);
		return (std::cout << "error connecting to server from client\n", -1);
	}
	std::cout << "client connected successfully to port - " << servInstance.getPort() << " \n";
	
	// add to epoll (here?)
	
	//// send message to server socket 
	//std::string message = "GET / HTTP/1.1\r\nHost: " + servInstance.getHost() + "\r\nConnection: close\r\n\r\n";
	//if (send(_sockfd, message.c_str(), message.size(), 0) < 0)
	//{
	//	close(_sockfd);
	//	return (std::cout << "error sending from client\n", -1);
	//}

	return 1;
}
