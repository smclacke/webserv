/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 16:32:36 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Socket::Socket() {}

Socket::Socket(eSocket type)
{
	if (type == eSocket::Client)
	{
		// do client socket things
	}
	else
	{
		// do server socket things
	}
}

Socket::Socket(const Webserv &servers)
{
	(void)servers; // wll actually use ... somehow sometime

	/*
		for (servers[i]) i < getServerCount
		{
			// get server + any other necessary data
			servers[i]._clientSocket -
			servers[i]._serverSocket -
			if (openSockets() < 0) // take servers[i]
				exit(EXIT_FAILURE); // need proper error handling
		}

	*/
}

Socket &Socket::operator=(const Socket &socket)
{
	if (this != &socket)
	{
		// clear all attributes || set to 0

		this->_sockfd = socket._sockfd;
		this->_connection = socket._connection;
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

void Socket::closeSockets()
{
	close(this->_connection);
	close(this->_sockfd);
}

// handling multiple connections
// opening clientSocket + serverSocket, then pass to epoll()
int Socket::openSockets()
{
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
