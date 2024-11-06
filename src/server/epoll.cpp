/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 17:55:25 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Epoll::Epoll() : _epfd(0), _serverfd(0), _clientfd(0), _numEvents(MAX_EVENTS), _serveraddlen(0) {}


Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll &Epoll::operator=(const Epoll &epoll)
{
	if (this != &epoll)
	{
		this->_epfd = epoll._epfd;
		this->_serverfd = epoll._serverfd;
		this->_clientfd = epoll._clientfd;
		this->_numEvents = epoll._numEvents;
		this->_serveraddlen = epoll._serveraddlen;
		this->_serveraddr = epoll._serveraddr;
		this->_event = epoll._event;
		this->_events[MAX_EVENTS - 1] = epoll._events[MAX_EVENTS - 1];
	}
	return *this;
}

Epoll::~Epoll() 
{ 
	if (_serverfd)
		protectedClose(_serverfd);
	if (_clientfd)
		protectedClose(_clientfd);
	if (_epfd)
		protectedClose(_epfd);

	// clean up events etc?
}


/* methods */

void		Epoll::initEpoll()
{
	_epfd = epoll_create(10); // need positive num but basically ignored
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

void		Epoll::connectClient()
{
	if ((connect(_clientfd, (struct sockaddr*)&_serveraddr, _serveraddlen)))
	{
		if (errno != EINPROGRESS)
		{
			protectedClose(_clientfd);
			protectedClose(_serverfd);
			protectedClose(_epfd);
			throw std::runtime_error("Failed to connect client socket to server\n");
		}
	}
}

// read data from client socket

// send response to client


void		Epoll::monitor(Socket &server, Socket &client)
{
	/* Server socket */
	setServerfd(server.getSockfd());
	setClientfd(client.getSockfd());
	setServeraddlen(server.getAddrlen());
	setServeraddr(server.getSockaddr());
	_event = addSocketEpoll(_serverfd, _epfd, eSocket::Server);
	
	/* Client socket */
	connectClient();
	std::cout << "Client connected to server successfuly \n";
	_event = addSocketEpoll(_clientfd, _epfd, eSocket::Client);

	while (true)
	{
		_numEvents = epoll_wait(_epfd, _events, 10, -1);
		if (_numEvents == -1)
			throw std::runtime_error("epoll_wait failed\n");

		for (int i = 0; i < _numEvents; ++i)
		{
			// accept incoming connection on server socket
			if (_events[i].data.fd == _serverfd)
			{
				socklen_t	newClientlen = client.getAddrlen();
				struct sockaddr_in	newClientaddr = client.getSockaddr();
				int newConnection = accept(_serverfd, (struct sockaddr *)&newClientaddr, &newClientlen);
				if (newConnection < 0)
				{
					std::cerr << "Error accepting new connection\n";
					continue ;
				}
				else 
				{
					setNonBlocking(newConnection);
					addConnectionEpoll(newConnection, _epfd, _event);
					std::cout << "Successfully made connection\n";
				}
			}
			else if (_events[i].events & EPOLLIN)
			{
				// read data from client socket
				char	buffer[1024];
				int		bytesRead = read(_events[i].data.fd, buffer, sizeof(buffer) - 1);
				
				if (bytesRead == -1)
				{
					protectedClose(_events[i].data.fd);
					throw std::runtime_error("Reading from client socket failed\n");
				}
				else if (bytesRead == 0)
				{
					protectedClose(_events[i].data.fd);
					std::cout << "Client disconnected\n";
				}
				else
				{
					buffer[bytesRead] = '\0';
					std::cout << "Server received " << buffer << "\n";
					switchOUTMode(_events[i].data.fd, _epfd, _event);
				}
			}
			else if (_events[i].events & EPOLLOUT)
			{
				// send response to client
				// take parsed <response> from HTTP shizzle from joolioos (definitely for non default)
				const char	*response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
				ssize_t	bytesWritten = write(_events[i].data.fd, response, strlen(response));
				if (bytesWritten == -1)
				{
					std::cerr << "Write to client failed\n";
					continue ;
				}
				std::cout << "Client sent message to server: " << response << "\n\n\n";
				switchINMode(_events[i].data.fd, _epfd, _event);
				protectedClose(_events[i].data.fd);
			}
		}
	}
	closeDelete(_serverfd, _epfd);
	closeDelete(_clientfd, _epfd);
	std::cout << "\nClosed server socket and deleted from Epoll\n";
}


/* getters */
int				Epoll::getEpfd() const
{
	return this->_epfd;
}

int				Epoll::getServerfd() const
{
	return this->_serverfd;
}

int				Epoll::getClientfd() const
{
	return this->_clientfd;
}

int				Epoll::getNumEvents() const
{
	return this->_numEvents;
}

socklen_t		Epoll::getServeraddlen() const
{
	return this->_serveraddlen;
}

struct sockaddr_in	Epoll::getServeraddr() const
{
	return this->_serveraddr;
}

/* setters */
void			Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void			Epoll::setServerfd(int fd)
{
	this->_serverfd = fd;
}

void			Epoll::setClientfd(int fd)
{
	this->_clientfd = fd;
}

void			Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}

void			Epoll::setServeraddlen(socklen_t addlen)
{
	this->_serveraddlen = addlen;
}

void			Epoll::setServeraddr(struct sockaddr_in addr)
{
	this->_serveraddr = addr;
}
