/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/28 17:09:03 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include "../../include/web.hpp"


// Webserv - > <vector> _servers - > _serverName, _host etc..

/* constructors */

Socket::Socket() {}

Socket::Socket(const Socket &socket)
{
	*this = socket;
}

Socket::Socket(const Webserv &servers)
{
	(void) servers; // wll actually use ... somehow sometime

	if (openSockets() < 0)
		exit(EXIT_FAILURE); // need proper error handling?
}

Socket	&Socket::operator=(const Socket &socket)
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

void	Socket::closeSockets()
{
	close(this->_connection);
	close(this->_sockfd);
}


// will need to loop socketsSSS - std::vector<Server> _servers; I THINK
// handling multiple connections too... right now one int but needs to become array/vector of ints

int		Socket::openSockets()
{

	// create socket (IPv4, TCP)
	_sockfd = socket(AF_INET, SOCK_STREAM, 0); // listening socket
	if (_sockfd == -1)
		return (std::cout << "failed to create socket\n", -1);
	//std::cout << "successfully created socket\n";


	// listen to port 9999 on any address
	// htons to convert a number to network byte order
	// address - location - path - root etc, same stuff?
	// webserv->server->location->client_body_buffer_size - what do we do with you?
	
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY;
	_sockaddr.sin_port = htons(9999); 			// config._servers._port

	// config._servers._host - don't know when/where this comes in but part of the config

	if (bind(_sockfd, (struct sockaddr*)&_sockaddr, sizeof(_sockaddr)) < 0)
		return (std::cout << "failed to bind to port 9999\n", -1);
	//std::cout << "binding to port 9999 successful\n";


	// start listening, hold at most 10 connections in the queue
	if (listen(_sockfd, 10) < 0)
		return (std::cout << "failed to listen on socket\n", -1);
	std::cout << "listening successfully\n";



	//std::vector<Server> _servers;
// not sure where/when you need to be

	// grab a connection from the queue
	_addrlen = sizeof(_sockaddr);
	_connection = accept(_sockfd, (struct sockaddr*)&_sockaddr, (socklen_t*)&_addrlen);
	if (_connection < 0)
		return (std::cout << "failed to grab connection\n", -1);
		
	//setNonblocking(connection)

	std::cout << "successfully made connection\n";
	

// unncessary bit, leaving for testing
	
	// read fromm the connection
	char	buffer[100];
	size_t	bytesRead = read(_connection, buffer, 100);
	(void) bytesRead;
	std::cout << "message from connection: " << buffer;


	// send message to the connection
	std::string	response = "nice chatting with you connection :)";
	send(_connection, response.c_str(), response.size(), 0);


	return 1; // success :)
}

