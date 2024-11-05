/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 15:05:01 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Epoll::Epoll() {}


Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll &Epoll::operator=(const Epoll &epoll)
{
	if (this != &epoll)
	{
		this->_epfd = epoll._epfd;
		// events...
	}
	return *this;
}

Epoll::~Epoll() {}


/* methods */

static std::string generateHttpResponse(const std::string &message)
{
	size_t	contentLength = message.size();
	std::ostringstream	response;
	response	<< "HTPP/1.1 200 OK\r\n"
				<< "Content-Type: text/plain\r\n"
				<< "Content-Length: " << contentLength << "\r\n"
				<< "Connection : close\r\n"
				<< "\r\n"
				<< message;
	
	return response.str();
}


int		Epoll::initEpoll()
{
	_epfd = epoll_create(1);
	if (_epfd < 0)
	{
		std::cerr << "\n\n222: error creating epoll instance\n";
		// close sockets ? + other cleaning up
		// throw here? or in webser constructor?
		return -1;
	}
	std::cout << "\n\n222: successfully created Epoll instance\n";
	return 0; // success
}


int		Epoll::monitor(const Socket &client, const Socket &server)
{
	int		serverSockFd = server.getSockFd();
	socklen_t	serverAddrlen = server.getAddrlen();
	struct sockaddr_in	serverAddr = server.getSockaddr();


	// add server socket to epoll
	_event.events = EPOLLIN; // monitor incoming connections
	_event.data.fd = serverSockFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, serverSockFd, &_event) < 0)
	{
		std::cerr << "\n\n111: error adding server socket to epoll\n";
		// clean? throw?
		return -1;
	}
	std::cout << "\n\n111: successfully added server socket to epoll\n";

	while (true)
	{
		//_events[10]; // array holding events
		int		nfds = epoll_wait(_epfd, _events, 10, -1); // wait for events

		for (int i = 0; i < nfds; ++i)
		{
			if (_events[i].data.fd == serverSockFd)
			{
				// accept new connection
				int newConnection = accept(serverSockFd, (struct sockaddr *)&serverAddr, &serverAddrlen);
				if (newConnection < 0)
				{
					std::cerr << "error accepting new connection\n";
					continue ;
				}
				std::cout << "successfully made connection\n";
				
				// set new connection socket to non-blocking
				int	flag = fcntl(newConnection, F_GETFL, 0);
				fcntl(newConnection, F_SETFL, flag | O_NONBLOCK);
				
				// add new connection to epoll instance
				_event.events = EPOLLIN | EPOLLET; // edge-triggered (wit?)
				_event.data.fd = newConnection;
				if (epoll_ctl(_epfd, EPOLL_CTL_ADD, newConnection, &_event) < 0)
				{
					std::cerr << "error adding connection to epoll\n";
					close(newConnection);
				}
			}
			else
			{
				// handle incoming data from client connection
				int		clientSockFd = client.getSockFd(); // why not this?
				//int			clientSockFd = _events[i].data.fd;
				char		buffer[1024];
				ssize_t 	bytesRead = read(clientSockFd, buffer, sizeof(buffer) - 1);
				if (bytesRead > 0)
				{
					buffer[bytesRead] = '\0';
					std::cout << "received request: " << buffer << "\n";
					
					// send HTTP response
					std::string	response = generateHttpResponse("here, have a response");
					send(clientSockFd, response.c_str(), response.size(), 0);
				}
				else
				{
					if (bytesRead < 0)
						std::cerr << "error reading from client\n";
					close(clientSockFd);
					epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSockFd, nullptr);
				}
			}
		}
	}
	close(_epfd); // here? maybe not cause many servers, do this somewhere else
	return 0; // success
}
