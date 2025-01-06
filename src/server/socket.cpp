/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/06 18:27:41 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/socket.hpp"

/* constructors */
Socket::Socket() {}

Socket::Socket(const Server &servInstance) : _sockfd(-1), _maxConnections(10), _reuseaddr(1), _flags(0)
{
	openServerSocket(servInstance);
	std::cout << "Server socket setup successful\n\n";
}

Socket::~Socket()
{
	protectedClose(_sockfd);
}

/* methods */
void Socket::openServerSocket(const Server &servInstance)
{
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("socket()\n");

	_flags = fcntl(_sockfd, F_GETFL, 0);
	if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("setting socket to nonblocking\n");
	}

	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
		throw std::runtime_error("setsockopt()\n");

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
		if (errno == EADDRINUSE)
		{
			protectedClose(_sockfd);
			throw std::runtime_error("address(port) already in use");
		}
		protectedClose(_sockfd);
		throw std::runtime_error("bind()\n");
	}

	if (listen(_sockfd, _maxConnections) < 0)
	{
		protectedClose(_sockfd);
		throw std::runtime_error("listen()\n");
	}
	setSockaddr(_sockaddr);
	setAddrlen(_addrlen);

	std::cout << "Server: " << servInstance.getServerName() << "\n";
	std::cout << "Port: " << servInstance.getPort() << "\n";
	std::cout << "Host: " << servInstance.getHost().c_str() << " \n";
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
