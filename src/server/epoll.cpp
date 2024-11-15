/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/15 15:36:05 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Epoll::Epoll() : _epfd(0), _numEvents(MAX_EVENTS) {}


Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll &Epoll::operator=(const Epoll &epoll)
{
	if (this != &epoll)
	{
		this->_epfd = epoll._epfd;
		this->_numEvents = epoll._numEvents;
	}
	return *this;
}

Epoll::~Epoll() 
{ 
	// for vec fds...
	//{

		//if (_serverfd)
		//	protectedClose(_serverfd);
		//if (_clientfd)
		//	protectedClose(_clientfd);
	//}
	
	if (_epfd)
		protectedClose(_epfd);

	// freeaddrinfo
	// more clean
}


/* methods */

void		Epoll::initEpoll()
{
	_epfd = epoll_create(10);
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

void		Epoll::connectClient(t_fds fd)
{
	if ((connect(fd._clientfd, (struct sockaddr*)&fd._serveraddr, fd._serveraddlen)))
	{
		if (errno != EINPROGRESS)
		{
			protectedClose(fd._clientfd);
			protectedClose(fd._serverfd);
			protectedClose(_epfd);
			throw std::runtime_error("Failed to connect client socket to server\n");
		}
	}
}

void		Epoll::readClient(t_fds fd, int i)
{
	char	buffer[1024];
	int		bytesRead = read(fd._events[i].data.fd, buffer, sizeof(buffer) - 1);
	
	if (bytesRead == -1)
	{
		protectedClose(fd._events[i].data.fd);
		throw std::runtime_error("Reading from client socket failed\n");
	}
	else if (bytesRead == 0)
	{
		protectedClose(fd._events[i].data.fd);
		std::cout << "Client disconnected\n";
	}
	else
	{
		buffer[bytesRead] = '\0';
		std::cout << "Server received " << buffer << "\n";
		switchOUTMode(fd._events[i].data.fd, _epfd, fd._event);
	}
}

void		Epoll::sendResponse(t_fds fd, int i)
{
	const char	*response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	
	ssize_t	bytesWritten = write(fd._events[i].data.fd, response, strlen(response));

	if (bytesWritten == -1)
	{
		std::cerr << "Write to client failed\n";
		return ;
	}

	std::cout << "Client sent message to server: " << response << "\n\n\n";
	switchINMode(fd._events[i].data.fd, _epfd, fd._event);
	protectedClose(fd._events[i].data.fd);
}

void		Epoll::serverSockConnect(Socket &server, t_fds fd)
{
	fd._newaddlen = server.getAddrlen();
	fd._newaddr = server.getSockaddr();
	fd._newfd = accept(fd._serverfd, (struct sockaddr *)&fd._newaddr, &fd._newaddlen);
	if (fd._newfd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return ;
	}
	else 
	{
		setNonBlocking(fd._newfd);
		std::cout << "Successfully made connection\n";
		addConnectionEpoll(fd._newfd, _epfd, fd._event);
	}
}

// i for fd index, j for event index
void		Epoll::monitor(Socket &server, size_t i)
{
	/* Client socket */
	connectClient(_fds[i]);
	std::cout << "Client connected to server successfully \n";
	_fds[i]._event = addSocketEpoll(_fds[i]._clientfd, _epfd, eSocket::Client);
	while (true)
	{
		_numEvents = epoll_wait(_epfd, _fds[i]._events, 10, -1);
		if (_numEvents == -1)
			throw std::runtime_error("epoll_wait failed\n");

		for (int j = 0; j < _numEvents; ++j)
		{
			if (_fds[i]._events[j].data.fd == _fds[i]._serverfd)
				serverSockConnect(server, _fds[i]);
			else if (_fds[i]._events[j].events & EPOLLIN)
				readClient(_fds[i], j);
			else if (_fds[i]._events[j].events & EPOLLOUT)
				sendResponse(_fds[i], j);
		}
	}
	// move this to after all servers have been monitored, clean up at end?
	closeDelete(_fds[i]._serverfd, _epfd);
	closeDelete(_fds[i]._clientfd, _epfd);
	std::cout << "\nClosed server socket and deleted from Epoll\n";
}



/* getters */
int					Epoll::getEpfd() const
{
	return this->_epfd;
}


std::vector<t_fds>	Epoll::getAllFds() const
{
	return this->_fds;
}

t_fds				Epoll::getFd(size_t i) const
{
	return this->_fds[i];
}

int					Epoll::getNumEvents() const
{
	return this->_numEvents;
}


/* setters */
void				Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void				Epoll::setFd(t_fds fd)
{
	this->_fds.push_back(fd);
}

void				Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}
