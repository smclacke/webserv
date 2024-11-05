/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 19:45:47 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Socket::Socket() {}

Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10), _connection(0), _reuseaddr(1), _flags(0)
{
	if (type == eSocket::Server)
	{
		if (openServerSocket(servInstance) < 0)
			std::cout << "\nopen server socket error\n\n"; // add throw
		else
			std::cout << "\nserver socket setup successful\n\n";

	}
	else if (type == eSocket::Client)
	{
		if (openClientSocket(servInstance) < 0)
			std::cout << "\nopen client socket error\n\n"; // add throw
		else
			std::cout << "\nclient socket setup successful\n\n";

	}
	//else
		// throw - incorrect contructor parameter
}

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
	setSockFd(_sockfd);
	setSockaddr(_sockaddr);
	_addrlen = sizeof(_sockaddr);
	setAddrlen(_addrlen);
	
	// bind
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
	std::cout << "\nlistening successfully on port - " << servInstance.getPort() << " \n";

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
	setSockFd(_sockfd);

	// set to non-blocking socket mode
	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting client socket to nonblocking\n", -1);
	}
	
	setSockaddr(_sockaddr);
	_addrlen = sizeof(_sockaddr);
	setAddrlen(_addrlen);

	// attempt to connect
	if ((connect(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen)) < 0)
	{
		close(_sockfd);
		return (std::cout << "error connecting to server from client\n", -1);
	}
	std::cout << "\nclient connected successfully to port - " << servInstance.getPort() << " \n";

	return 1;
}


/* getters */

int	Socket::getSockFd() const
{
	return this->_sockfd;
}

int	Socket::getConnection() const
{
	return this->_connection;
}

struct sockaddr_in	Socket::getSockaddr() const
{
	return this->_sockaddr;
}

socklen_t	Socket::getAddrlen() const
{
	return this->_addrlen;
}

/* setters */

void			Socket::setSockFd(int fd)
{
	this->_sockfd = fd;
}

void		Socket::setNewConnection(int &connection)
{
	this->_connection = connection;
}

void		Socket::setSockaddr(struct sockaddr_in &sockaddr)
{
	this->_sockaddr = sockaddr;
}

void		Socket::setAddrlen(socklen_t &addrlen)
{
	this->_addrlen = addrlen;
}
