/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 17:53:22 by smclacke      ########   odam.nl         */
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
	size_t	contentLength = message.size(); // lwngth
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
	_epfd = epoll_create(1); // 0
	if (_epfd < 0)
	{
		std::cerr << "\nerror creating epoll instance\n";
		// close sockets ? + other cleaning up
		// throw here? or in webser constructor?
		return -1;
	}
	std::cout << "\nsuccessfully created Epoll instance\n";
	return 0; // success
}


int		Epoll::monitor(Socket &server, Socket &client)
{
	int		serverSockFd = server.getSockFd();

	socklen_t	serverAddrlen = server.getAddrlen();
	struct sockaddr_in	serverAddr = server.getSockaddr();

	// add server socket to epoll
	//_event.events = EPOLLIN; // monitor incoming connections
	//_event.data.fd = serverSockFd;
	
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = serverSockFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, serverSockFd, &event) < 0)
	{
		std::cerr << "\nerror adding server socket to epoll\n";
		// clean? throw?
		return -1;
	}
	std::cout << "\nsuccessfully added server socket to epoll\n";


	struct epoll_event events[10];
	while (true)
	{
		int numEvents = epoll_wait(_epfd, events, 10, -1); // wait for events
		perror("");
		if (numEvents == -1)
		{
			std::cout << "fucked\n";
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < numEvents; ++i)
		{
			if (events[i].data.fd == serverSockFd)
			{
				// accept new connection // LOOP here till the fully request has been received
				serverAddrlen = server.getAddrlen();
				serverAddr = server.getSockaddr();
				int newConnection = accept(serverSockFd, (struct sockaddr *)&serverAddr, &serverAddrlen);
				server.setNewConnection(newConnection);

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
				event.events = EPOLLIN;
				event.data.fd = newConnection;
				if (epoll_ctl(_epfd, EPOLL_CTL_ADD, newConnection, &event) < 0)
				{
					std::cerr << "error adding connection to epoll\n";
					close(newConnection);
				}
				std::cout << "successfully added connection to epoll\n";
			}
			else
			{
				// handle incoming data from client connection
				int			clientSockFd = events[i].data.fd;
				char		buffer[1024];
				std::string	request;
				ssize_t		bytesRead = 0;

				while ((bytesRead = read(clientSockFd, buffer, sizeof(buffer) - 1)) > 0)
				{
					buffer[bytesRead] = '\0';
					request.append(buffer);
				}
				if (bytesRead < 0)
				{
					std::cerr << "error reading from client\n";
					close(clientSockFd);
					epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSockFd, nullptr);
					return -1;
				}

				std::cout << "received reuqest: " << request << "\n";

				std::string response = generateHttpResponse("HELLO WORLLLLLD");
				send(clientSockFd, response.c_str(), response.size(), 0);
				close(clientSockFd);
				epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSockFd, nullptr);
			}
		}
	}
	return 0; // success
}
