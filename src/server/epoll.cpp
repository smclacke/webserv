/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 20:30:32 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

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
		this->_op = epoll._op;
		this->_numEvents = epoll._numEvents;
		// timeout
	}
	return *this;
}

Epoll::~Epoll() {} // delete epoll socket stuff?


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

void		Epoll::initEpoll()
{
	_epfd = epoll_create(1);
	if (_epfd < 0)
	{
		// close sockets ? + other cleaning up?
		throw std::runtime_error("Error creating Epoll instance\n");
	}
	std::cout << "Successfully created Epoll instance\n";
}

static struct epoll_event addSocketEpoll(int sockfd, int epfd)
{
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = sockfd;
	
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		throw std::runtime_error("Error adding server socket to epoll\n");
	}
	std::cout << "Successfully added server socket to epoll\n";
	return event;
}

static void		addConnectionEpoll(int connection, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = connection;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, connection, &event) < 0)
	{
		close(connection);
		throw std::runtime_error("Error adding connection to epoll\n");
	}
	std::cout << "Successfully added connection to epoll\n";
}

static	void	setNonBlocking(int connection)
{
	int	flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void		Epoll::monitor(Socket &server, Socket &client)
{
	int		serverSockfd = server.getSockFd();
	socklen_t	serverAddrlen = server.getAddrlen();
	struct sockaddr_in	serverAddr = server.getSockaddr();

	// add server socket to epoll
	struct epoll_event event = addSocketEpoll(serverSockfd, _epfd);

	struct epoll_event events[10];

	while (true)
	{
		int numEvents = epoll_wait(_epfd, events, 10, -1); // wait for events

		for (int i = 0; i < numEvents; ++i)
		{
			if (events[i].data.fd == serverSockfd)
			{
				// accept new connection // LOOP here till the full request has been received
				serverAddrlen = server.getAddrlen();
				serverAddr = server.getSockaddr();
				int newConnection = accept(serverSockfd, (struct sockaddr *)&serverAddr, &serverAddrlen);
				server.setNewConnection(newConnection);

				if (newConnection < 0)
				{
					std::cerr << "Error accepting new connection\n";
					continue ;
				}
				std::cout << "Successfully made connection\n";

				setNonBlocking(newConnection);
				addConnectionEpoll(newConnection, _epfd, event);

				//if ((connect()))
			}
			else
			{
				// !! here to me, there is a disconnect between clientserver here and in openclientsocket
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
					close(clientSockFd);
					epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSockFd, nullptr);
					throw std::runtime_error("Error reading from client\n");
				}

				std::cout << "received reuqest: " << request << "\n";

				std::string response = generateHttpResponse("HELLO WORLLLLLD");
				send(clientSockFd, response.c_str(), response.size(), 0);
				close(clientSockFd);
				epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSockFd, nullptr);
			}
		}
	}
	// what about closing and deleting serversockfd??
}
