/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/04 14:22:46 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/socket.hpp"

/* constructors */
Socket::Socket() {}

Socket::Socket(const Server &servInstance) : _maxConnections(10), _reuseaddr(1), _flags(0)
{
	openServerSocket(servInstance);
	std::cout << "Server socket setup successful\n\n";
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
		this->_sockaddr = socket._sockaddr;
		this->_addrlen = socket._addrlen;
		this->_reuseaddr = socket._reuseaddr;
		this->_flags = socket._flags;
	}
	return *this;
}

Socket::~Socket()
{
	std::cout << "Socket destructor called" << std::endl;
	protectedClose(_sockfd);
}

/* methods */
/** @todo if this server fails, continue and make the rest? if second(...) server fails, need to clean up rest
 * if we throw since failure to set up server is pretty bad, need to clean up already made server stuff?
 */
void		Socket::openServerSocket(const Server &servInstance)
{
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Error socketing sock\n");

	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("Error setting nonblocking\n");
	}

	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
		throw std::runtime_error("Error setsockopt sock\n");

	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, servInstance.getHost().c_str(), &_sockaddr.sin_addr) <= 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("Invalid IP address format\n");
	}
	_sockaddr.sin_port = htons(servInstance.getPort());
	_addrlen = sizeof(_sockaddr);

	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen) < 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("Error binding sock\n");
	}

	if (listen(_sockfd, _maxConnections) < 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("Error listening for connections\n");
	}
	setSockaddr(_sockaddr);
	setAddrlen(_addrlen);

	std::cout << "Listening on port - " << servInstance.getPort() << "\n";
	std::cout << "Host address - " << servInstance.getHost().c_str() << " \n";
}

/* getters */
int Socket::getSockfd() const
{
	return this->_sockfd;
}

struct sockaddr_in Socket::getSockaddr() const
{
	return this->_sockaddr;
}

socklen_t Socket::getAddrlen() const
{
	return this->_addrlen;
}

/* setters */
void Socket::setSockfd(int fd)
{
	this->_sockfd = fd;
}

void Socket::setSockaddr(struct sockaddr_in &sockaddr)
{
	this->_sockaddr = sockaddr;
}

void Socket::setAddrlen(socklen_t &addrlen)
{
	this->_addrlen = addrlen;
}
