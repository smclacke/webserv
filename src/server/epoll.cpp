/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/18 17:07:16 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

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

void		Epoll::clientStatus(t_fds fd)
{
	auto now = std::chrono::steady_clock::now();
	
	for (auto it = fd._clientStatus.begin(); it != fd._clientStatus.end();)
	{
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
		if (elapsed.count() >= (TIMEOUT / 1000))
		{
			protectedClose(it->first);
			// remove fd from epoll
			// delete client(it->first)
			// it = _clientStatus.erase(it);
		}
		else
			it++;
	}
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

void		Epoll::readIncomingMessage(t_fds fd, int i)
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

void		Epoll::sendOutgoingResponse(t_fds fd, int i)
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

void		Epoll::makeNewConnection(std::shared_ptr<Socket> &server, t_fds fd)
{
	socklen_t newaddlen = server->getAddrlen();
	struct sockaddr_in newaddr = server->getSockaddr();
	int newfd = accept(fd._serverfd, (struct sockaddr *)&newaddr, &newaddlen);
	if (newfd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return ;
	}
	else 
	{
		setNonBlocking(newfd);
		std::cout << "\nNew connection made\n";
		addConnectionEpoll(newfd, _epfd, fd._event);
	}
	
	// handle incoming in vector of connections

	//fd._connection._conAddLen = server->getAddrlen();
	//fd._newaddr = server->getSockaddr();
	//fd._newfd = accept(fd._serverfd, (struct sockaddr *)&fd._newaddr, &fd._newaddlen);
	//if (fd._newfd < 0)
	//{
	//	std::cerr << "Error accepting new connection\n";
	//	return ;
	//}
	//else 
	//{
	//	setNonBlocking(fd._newfd);
	//	std::cout << "Successfully made new connection\n";
	//	addConnectionEpoll(fd._newfd, _epfd, fd._event);
	//}
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
