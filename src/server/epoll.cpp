/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/06 14:13:43 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

/* constructors */
Epoll::Epoll() : _epfd(0), _numEvents(MAX_EVENTS)
{
	setEventMax();
}

Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll	&Epoll::operator=(const Epoll &epoll)
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
	if (_epfd > 0)
		protectedClose(_epfd);
}

/* methods */
void	Epoll::initEpoll()
{
	_epfd = epoll_create(10);
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
}

void	Epoll::handleRead(t_clients &client)
{
	char buffer[READ_BUFFER_SIZE];
	size_t bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));

	client._clientState = clientState::READING;
	bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
	
	// Error
	if (bytesRead < 0)
	{
		std::cerr << "Reading from client connection failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		return ;
	}
	
	// Disconnected
	else if (bytesRead == 0)
	{
		//std::cout << "Client disconnected\n";
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return ;
	}

	buffer[READ_BUFFER_SIZE - 1] = '\0';
	std::string buf = buffer;
	client._requestClient.append(buf);

	// FInished
	if (client._requestClient.find("\r\n\r\n") != std::string::npos)
	{
		client._clientState = clientState::READY;
		client._connectionClose = false;
		return ;
	}
	client._connectionClose = false;
}

void	Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	// Get the request response if we are here for the first time
	if (client._responseClient.msg.empty())
	{
		client._responseClient = server._server->handleRequest(client._requestClient);
		client._clientState = clientState::WRITING;
		client._requestClient.clear();
	}

	ssize_t		leftover;
	ssize_t		sendlen = WRITE_BUFFER_SIZE;
	leftover = client._responseClient.msg.size() - client._write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;

	ssize_t bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, leftover, 0);
	
	// Error
	if (bytesWritten < 0)
	{
		std::cerr << "Write to client failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		return ;
	}

	// Disconnected
	else if (bytesWritten == 0)
	{
		//std::cout << "Client disconnected\n";
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return ;
	}

	client._write_offset += bytesWritten;
	
	// FInished
	if (client._write_offset >= client._responseClient.msg.length())
	{
		if (client._responseClient.keepAlive == false)
		{
			client._clientState = clientState::CLOSE;
			client._connectionClose = true;
		}
		client._clientState = clientState::READY;
		client._connectionClose = false;
		client._write_offset = 0;
		client._responseClient.msg.clear();
		return ;
	}
	client._connectionClose = false;
}

/** @todo this needs to be cleaned up + ensure it works with J new stuff */
void	Epoll::handleFile(t_clients &client)
{
		/** @todo this.. OR add it in readFile GET.cpp */
		//addToEpoll(client._responseClient.readFd);
		
		ssize_t		bytesSend;
		char		buffer[READ_BUFFER_SIZE];
		// i guess im not reading here anymore?
		ssize_t		bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE - 1);

		// Error
		if (bytesRead < 0)
		{
			std::cerr << "Reading from pipe failed\n";
			client._clientState = clientState::ERROR;
			client._connectionClose = true;
			return ;
		}

		// Nothing to read -> we are done
		else if (bytesRead == 0)
		{
			client._readingFile = false;
			client._clientState = clientState::READY;
			protectedClose(client._responseClient.readFd);
			if (client._responseClient.pid != -1)
			{
				int status;
				waitpid(client._responseClient.pid, &status, 0);
			}
			client._responseClient.readFd = -1;
			client._responseClient.pid = -1;
			if (client._responseClient.keepAlive == false)
			{
				client._clientState = clientState::CLOSE;
				client._connectionClose = true;
			}
			return ;
		}
		buffer[READ_BUFFER_SIZE - 1] = '\0';
		if (bytesRead == READ_BUFFER_SIZE - 1) // we are not done
		{
			std::cout << "WE READ:--" << buffer << std::endl;
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				std::cout << "HELPP\n";
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					return;
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return;
			}
			std::cout << "BYTES SEND: " << bytesSend << std::endl;
		}
		else if (bytesRead < READ_BUFFER_SIZE) // we need to send and then we are done
		{
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					return ;
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return ;
			}
			client._readingFile = false;
			client._clientState = clientState::READY;
			close(client._responseClient.readFd);
			if (client._responseClient.pid != -1)
			{
				int status;
				waitpid(client._responseClient.pid, &status, 0);
			}
			client._responseClient.readFd = -1;
			client._responseClient.pid = -1;
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;
			return ;
		}
}

void	Epoll::makeNewConnection(int fd, t_serverData &server)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	int clientfd = accept(fd, (struct sockaddr *)&clientAddr, &addrLen);
	if (clientfd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return ;
	}
	else
	{
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen);
		addToEpoll(clientfd);
	}
}

void	Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			if (event.events & EPOLLIN)
				makeNewConnection(fd, serverData);
		}
		for (auto &client : serverData._clients)
		{
			if (fd == client._fd)
			{
				if (event.events & EPOLLIN)
				{
					handleRead(client);
					if (client._clientState == clientState::READY)
					{
						client._clientState = clientState::BEGIN;
						modifyEvent(client._fd, EPOLLOUT);
						updateClientClock(client);
					}
				}
				if (event.events & EPOLLOUT)
				{
					//if (client._readingFile == false)
					//	handleWrite(serverData, client);
					//if (client._readingFile == true)
						//handleFile(client);
						
					handleBigWrite(serverData, client);
					if (client._clientState == clientState::READY)
					{
						client._clientState = clientState::BEGIN;
						modifyEvent(client._fd, EPOLLIN);
						updateClientClock(client);
					}
				}
				if (event.events & EPOLLHUP)
				{
					std::cout << "Epoll: EPOLLHUP\n";
					client._connectionClose = true;
				}
				if (event.events & EPOLLRDHUP)
				{
					std::cout << "Epoll: EPOLLRDHUP\n";
					client._connectionClose = true;
				}
				if (event.events & EPOLLERR)
				{
					std::cout << "EPoll: EPOLLERR\n";
					client._connectionClose = true;
				}
				if (client._connectionClose == true)
					handleClientClose(serverData, client);
			}
		}
	}
	
}

/* getters */
int	Epoll::getEpfd() const
{
	return this->_epfd;
}

int	Epoll::getNumEvents() const
{
	return this->_numEvents;
}

std::vector<t_serverData>	&Epoll::getAllServers()
{
	return this->_serverData;
}

std::shared_ptr<Server>	Epoll::getServer(size_t i)
{
	return this->_serverData[i]._server;
}

std::vector<epoll_event>	&Epoll::getAllEvents()
{
	return _events;
}

struct epoll_event	&Epoll::getEvent()
{
	return this->_event;
}

/* setters */
void	Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void	Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}

void	Epoll::setEventMax()
{
	_events.resize(MAX_EVENTS);
}

void	Epoll::setEvent(struct epoll_event &event)
{
	this->_event = event;
}

void	Epoll::setServer(std::shared_ptr<Server> server)
{
	t_serverData newServerData;
	newServerData._server = server;

	this->_serverData.push_back(newServerData);
}
