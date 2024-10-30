/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 18:38:22 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Socket::Socket() {}

Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10), _connection(0)
{
	
	if (type == eSocket::Client)
	{
		// if openSocket() < 0 throw
		openClientSocket(servInstance);

	}
	else if (type == eSocket::Server)
	{
		// if openSocket() < 0 throw
		openServerSocket(servInstance);

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
	//close(this->_connection); // need you?
	//close(this->_connections); // close the vector of connections
	close(this->_sockfd);
}


// !! if fail throw for like everything !!
int	Socket::openServerSocket(const Server &servInstance)
{

	// create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	// set non-blocking
	//

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY; // listen port 9999 on any address - change later
	_sockaddr.sin_port = servInstance.getPort();


	// bind
	bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));

	
	// listen
	listen(_sockfd, _maxConnections);
	
	// accept
	_addrlen = sizeof(_sockaddr);


	// breaks here

	_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addrlen);
	std::cout << "successfully made connection\n";


	// read fromm the connection
	char	buffer[100];
	size_t	bytesRead = read(_connection, buffer, 100);
	std::cout << "message from connection: " << buffer;
	std::cout << "bytesRead from connection: " << bytesRead << "\n";


	// send message to the connection
	std::string	response = "nice chatting with you connection :) \n";
	send(_connection, response.c_str(), response.size(), 0);

	
	return 1;

}


int Socket::openClientSocket(const Server &servInstance)
{
 // check - no bind? no accept? listen??
 
	// create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// set non-blocking for client too?

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY; // listen port 9999 on any address - change later
	_sockaddr.sin_port = servInstance.getPort();

	// listen?
	listen(_sockfd, _maxConnections);

	// accept
	_addrlen = sizeof(_sockaddr);
	_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addrlen);
	std::cout << "successfully made connection\n";


	// connect!


	return 1; // success :)
}
