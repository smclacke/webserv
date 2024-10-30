/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 17:39:04 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Socket::Socket() {}

Socket::Socket(const Server &serv_instance, eSocket type) : _connection(0), _maxConnections(10)
{
	
	if (type == eSocket::Client)
	{
		// if openSocket() < 0 throw
		openClientSocket(serv_instance);

	}
	else if (type == eSocket::Server)
	{
		// if openSocket() < 0 throw
		openServerSocket(serv_instance);

	}
	//else
		// throw
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
	// clear all attributes (e.g. _addrlen.clear())
	// || set back to 0
}


/* methods */

void Socket::closeSockets() // or socket for each instance of socket
{
	//close(this->_connection); // close the vector of connections
	close(this->_sockfd);
}


// !! if fail throw for like everything !!
int	Socket::openServerSocket(const Server &serv_instance)
{

	// create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	// set non-blocking
	//

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY; // listen port 9999 on any address - change later
	_sockaddr.sin_port = serv_instance.getPort();


	// bind
	bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));

	
	// listen
	for (size_t i = 0; i < _maxConnections; i++)
	{
		listen(_sockfd, _connections[i]);
		
		// grab one for queue: so yay or nee for loop?
		// accept
		_addrlen = sizeof(_sockaddr);
		_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addrlen);
		if (_connection >= 0)
			std::cout << "successfully made connection\n";
	}

}


int Socket::openClientSocket(const Server &serv_instance)
{
 // check - no bind? no accept?
 
	// socket
	// connect
	// listen
	


	// create socket (IPv4, TCP)
	_sockfd = socket(AF_INET, SOCK_STREAM, 0); // listening socket
	if (_sockfd == -1)
		return (std::cout << "failed to create socket\n", -1);
	// std::cout << "successfully created socket\n";

	// set socket to non-blocking

	// listen to port 9999 on any address
	// htons to convert a number to network byte order

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY;
	_sockaddr.sin_port = htons(9999);

	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr)) < 0)
		return (std::cout << "failed to bind to port 9999\n", -1);
	std::cout << "binding to port 9999 successful\n";

	// start listening, hold at most 10 connections in the queue
	if (listen(_sockfd, 10) < 0)
		return (std::cout << "failed to listen on socket\n", -1);
	std::cout << "listening successfully\n";

	// grab a connection from the queue
	_addrlen = sizeof(_sockaddr);
	_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addrlen);
	if (_connection < 0)
		return (std::cout << "failed to grab connection\n", -1);

	std::cout << "successfully made connection\n";

	// unncessary bit, leaving for testing

	// read fromm the connection
	// char	buffer[100];
	// size_t	bytesRead = read(_connection, buffer, 100);
	//(void) bytesRead;
	// std::cout << "message from connection: " << buffer;

	//// send message to the connection
	// std::string	response = "nice chatting with you connection :)";
	// send(_connection, response.c_str(), response.size(), 0);

	return 1; // success :)
}
