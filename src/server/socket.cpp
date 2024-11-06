/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 14:34:17 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Socket::Socket() {}

Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10), _connection(0), _reuseaddr(1), _flags(0)
{
	if (type == eSocket::Server)
	{
		openServerSocket(servInstance);
		std::cout << "Server socket setup successful\n";

	}
	else if (type == eSocket::Client)
	{
		openClientSocket(servInstance);
		std::cout << "Client socket setup successful\n";
		std::cout << "---------------------------------\n\n";

	}
	else
		throw std::runtime_error("Error invalid socket type argument passed\n");
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
void		Socket::openServerSocket(const Server &servInstance)
{
	// create listening socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Error socketing sock\n");

	// set to non-blocking socket mode
	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("Error setting nonblocking\n");
	}

	// to re-bind without TIME_WAIT issues
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
		throw std::runtime_error("Error setsockopt sock\n");

	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockaddr.sin_port = htons(servInstance.getPort());
	setSockfd(_sockfd);
	setSockaddr(_sockaddr);
	_addrlen = sizeof(_sockaddr);
	setAddrlen(_addrlen);
	
	// bind
	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("Error binding sock\n");
	}

	// listen for incoming connections, (set up queue for incoming connections)
	if (listen(_sockfd, _maxConnections) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("Error listening for connections\n");
	}
	std::cout << "Listening on port - " << servInstance.getPort() << " \n";

}


// A process that initiates a service request
void 		Socket::openClientSocket(const Server &servInstance)
{
	(void) servInstance; // remove if definitely dont need
	
	// create client socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Error socketing sock\n");

	// set to non-blocking socket mode
	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("Error setting client socket to nonblocking\n");
	}
}


/* getters */

int	Socket::getSockfd() const
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

std::string	Socket::getHost() const
{
	return this->_host;
}

/* setters */

void			Socket::setSockfd(int fd)
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

void		Socket::setHost(std::string host)
{
	this->_host = host;
}
