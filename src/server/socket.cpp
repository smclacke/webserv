/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/31 16:46:55 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Socket::Socket() {}

Socket::Socket(const Server &servInstance, eSocket type) : _maxConnections(10),  _reuseaddr(1)
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


// A process that waits passively for requests from clients, processes the work specified,
// and returns the result to the client that originated the request.
int	Socket::openServerSocket(const Server &servInstance)
{

	// create listening socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);
	
	// to re-bind iwthout TIME_WAIT issues
	setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)); // can fail, need to check return types

	// set non-blocking -> check the application of this
	int	flags = fcntl(_sockfd, F_GETFL, 0); // this can fail!
	fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK); // this can also fail


	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockaddr.sin_port = servInstance.getPort();


	// bind
	if (bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr)) < 0)
		return (std::cout << "error binding sock\n", -1);

	
	// listen, (set up queue for incoming connections)
	if (listen(_sockfd, 10) < 0)
		return (std::cout << "error listening for connections\n", -1);
	std::cout << "listening successfully \n";
		
	auto addrlen = sizeof(_sockaddr);

	// accept
	int	connection;
	if ((connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, (socklen_t *) &addrlen)) < 0)
		return (std::cout << "error  accepting connection\n", -1);
	std::cout << "successfully made connection\n";
	

	// read fromm the connection
	char	buffer[100];
	read(_connection, buffer, 100); // can read fail here?
	std::cout << "message read by server: " << buffer;
	
	
	// send message to the connection
	std::string	response = "response message :) \n";
	send(_connection, response.c_str(), response.size(), 0);
	std::cout << "sent: " << response << " from server socket\n";

	close(_connection);
	close(_sockfd);

	return 1;
}


// A process that initiates a service request.
// connect to server sockt instead of bind
int Socket::openClientSocket(const Server &servInstance)
{

	// create socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);


	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sockaddr.sin_port = servInstance.getPort();
	_addrlen = sizeof(_sockaddr);

	int status;
	if ((status = connect(_sockfd, (struct sockaddr *) &_sockaddr, _addrlen)) < 0)
		return (std::cout << "error connecting to server socket\n", -1);
	std::cout << "client connected successfully\n";
	
	char	buffer[100];
	std::string	message = "message from client";
	
	send(_sockfd, message.c_str(), message.size(), 0);
	std::cout << "sent: <" << message << "> from client socket\n";

	read(_sockfd, buffer, 100);
	std::cout << "message read by client: <" << buffer << ">";

	close(_sockfd);
	

	return 1;
}
