/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 13:47:50 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 15:25:17 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include "../../include/web.hpp"


/* constructors */

Socket::Socket() {}

// need? - maybe for giving sockets to epoll
Socket::Socket(const Socket &socket)
{
	*this = socket;
}


// server	serv_instance;

//while (vector<servers>)
	// serv_instance = getserver(i)
	//_sockaddr.sin_port = serv_instance.getPort
	// sockfd = socket etc...

	//THEN
	// while serv.connections....
/*
Server serv;

while (vector<servers>)
{
	serv = webserv.getserver(0);
	_sockaddr.sin_port =serv.getPort; 
}

*/

// loopServers
//  -> per server - opensockets 
//	-> while opening sockets, listen for connections
Socket::Socket(const Webserv &servers)
{
	(void) servers; // wll actually use ... somehow sometime

	/*
		for (servers[i])
		{
			// get server + any other necessary data
			if (openSockets() < 0) // take servers[i]
				exit(EXIT_FAILURE); // need proper error handling
		}
	
	*/

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



// handling multiple connections right now one int but needs to become array/vector of ints
int		Socket::openSockets()
{

	// create socket (IPv4, TCP)
	_sockfd = socket(AF_INET, SOCK_STREAM, 0); // listening socket
	if (_sockfd == -1)
		return (std::cout << "failed to create socket\n", -1);
	//std::cout << "successfully created socket\n";


// !!!! understand this reuseable/nonblocking shit but doesnt work yet
// allow socket descriptor to be reuseable
//int	on = 1;
//int	rc = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
//if (rc == -1)
//	return (std::cout << "failed to set socket option\n", -1);	


// set socket to be nonblocking, all sockets for incoming connections be nonblocking since 
// they inherit that state from the listening socket
// setNonblocking(connection)
//rc = ioctl(_sockfd, FIONBIO, (char *)&on); // FIONBIO?
//if (rc == -1)
//	return (std::cout << "failed to ioctl, setting socket to nonblocking\n", -1);	


	// listen to port 9999 on any address
	// htons to convert a number to network byte order
	// address - location - path - root etc, same stuff?
	// webserv->server->location->client_body_buffer_size - what do we do with you?

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY;
	_sockaddr.sin_port = htons(9999); 			// config._servers._port
	// config._servers._host - don't know when/where this comes in but part of the config


// !!!! understand this reuseable/nonblocking shit but doesnt work yet
//rc = bind(_sockfd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));
//if (rc == -1)
//	return (std::cout << "failed to bind to port\n", -1);

//rc = listen(_sockfd, 10); // why 32?
//	return (std::cout << "listening failed\n", -1);
	

///////////////// older version of binding + litening
	if (bind(_sockfd, (struct sockaddr*)&_sockaddr, sizeof(_sockaddr)) < 0)
		return (std::cout << "failed to bind to port 9999\n", -1);
	std::cout << "binding to port 9999 successful\n";


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

