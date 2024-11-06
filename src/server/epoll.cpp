/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 15:06:12 by smclacke      ########   odam.nl         */
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

Epoll::~Epoll() { close(_epfd); }


/* methods */

std::string Epoll::generateHttpResponse(const std::string &message)
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
	_epfd = epoll_create1(0); // can't use epoll_Create1 but for now i am mwhaha
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

struct epoll_event Epoll::addSocketEpoll(int sockfd, int epfd, eSocket type)
{
	struct epoll_event	event;
	event.data.fd = sockfd;
	std::string		sortSocket;

	if (type == eSocket::Server)
	{
		event.events = EPOLLIN;
		sortSocket = "Server";
	}
	else if (type == eSocket::Client)
	{
		event.events = EPOLLIN | EPOLLOUT;
		sortSocket = "Client";
	}
	else
		throw std::runtime_error("invalid socket type passed as argument\n");
		
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		close(sockfd);
		throw std::runtime_error("Error adding socket to epoll\n");
	}
	std::cout << "Successfully added " << sortSocket << " socket to epoll\n";
	return event;
}

 void		Epoll::addConnectionEpoll(int connection, int epfd, struct epoll_event event)
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

void		Epoll::setNonBlocking(int connection)
{
	int	flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void		Epoll::closeDelete(int fd, int epfd)
{
	close(fd);
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void		Epoll::switchReadMode(int fd, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		close (fd);
		throw std::runtime_error("Failed to modify client socket for reading\n");
	}
	std::cout << "Successfully modified client socket for reading\n";
}

void		Epoll::monitor(Socket &server, Socket &client)
{
	int		serverSockfd = server.getSockfd();
	int		clientSockfd = client.getSockfd();

	socklen_t	serverAddrlen = server.getAddrlen();
	struct sockaddr_in	serverSockaddr = server.getSockaddr();

	// add server socket to epoll
	struct epoll_event event = addSocketEpoll(serverSockfd, _epfd, eSocket::Server);

	// connect the client to the server
	if ((connect(clientSockfd, (struct sockaddr*)&serverSockaddr, serverAddrlen)))
	{
		if (errno != EINPROGRESS)
		{
			close (clientSockfd);
			close (serverSockfd);
			close (_epfd);
			throw std::runtime_error("Client connection to server failed\n");
		}
	}
	std::cout << "Client connected to server successfuly \n";

	// add client socket to epoll for monitoring 
	event = addSocketEpoll(clientSockfd, _epfd, eSocket::Client);

	struct epoll_event events[10];
	while (true)
	{
		int numEvents = epoll_wait(_epfd, events, 10, -1); // wait for events
		if (numEvents == -1)
			throw std::runtime_error("epoll_wait failed\n");

		for (int i = 0; i < numEvents; ++i)
		{
			if (events[i].data.fd == serverSockfd)
			{
				// accept incoming connection on server socket
				socklen_t			newClientlen = client.getAddrlen();
				struct sockaddr_in		newClientaddr = client.getSockaddr();
				int newConnection = accept(serverSockfd, (struct sockaddr *)&newClientaddr, &newClientlen);
				if (newConnection < 0)
				{
					std::cerr << "Error accepting new connection\n";
					continue ;
				}
				else 
				{
					setNonBlocking(newConnection);
					server.setNewConnection(newConnection);
					// adding new connection to epoll (monitoring read events)
					addConnectionEpoll(newConnection, _epfd, event);
					std::cout << "Successfully made connection\n";
				}
			}
			else if (events[i].data.fd == clientSockfd && events[i].events & EPOLLOUT)
			{
				// handle connection request for the client (write event)
				const	char *message = "Hello from Client side";
				ssize_t	bytesWritten = write(clientSockfd, message, strlen(message));
				if (bytesWritten == -1)
				{
					std::cerr << "Write to client failed\n";
					continue ;
				}
				std::cout << "Client sent message to server: " << message << "\n";
				
				switchReadMode(clientSockfd, _epfd, event);
			}
			else if (events[i].events & EPOLLIN)
			{
				// read data from socket
				char	buffer[1024];
				int		bytesRead = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
				
				if (bytesRead <= 0)
				{
					close (events[i].data.fd);
					std::cout << "Client disconnected\n";
				}
				else
				{
					buffer[bytesRead] = '\0';
					std::cout << "Server received " << buffer << "\n";
				}
			}
		}
	}
	closeDelete(serverSockfd, _epfd);
	std::cout << "\nClosed server socket and deleted from Epoll\n";
}
