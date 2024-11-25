/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/25 13:56:34 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

/**
 * @todo destruction + clean up
 * @todo clientStatus delete
 */

/* constructors */
Epoll::Epoll() : _epfd(0), _numEvents(MAX_EVENTS)
{
	setEventMax();
}


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
	std::cout << "epoll destructor called\n";
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

void		Epoll::resizeEventBuffer(int size)
{
	_events.resize(size);
}

void		Epoll::clientTime(t_serverData server)
{
	auto now = std::chrono::steady_clock::now();
	
	for (auto it = server._clientTime.begin(); it != server._clientTime.end();)
	{
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
		if (elapsed.count() >= (TIMEOUT / 1000))
		{
			std::cout << "Client timed out\n";
			protectedClose(it->first);
			// remove server from epoll
			// delete client(it->first)
			// it = _clientTime.erase(it);
		}
		else
			it++;
	}
}

void Epoll::connectClient(t_serverData server)
{
	sockaddr_in serverSockAddr = server._server->getServerSocket()->getSockaddr();
	if ((connect(server._server->getClientSocket()->getSockfd(),
				 (struct sockaddr *)&serverSockAddr,
				 server._server->getServerSocket()->getAddrlen())))
	{
		if (errno != EINPROGRESS)
		{
			protectedClose(server._server->getClientSocket()->getSockfd());
			protectedClose(server._server->getServerSocket()->getSockfd());
			protectedClose(_epfd);
			throw std::runtime_error("Failed to connect client socket to server\n");
		}
	}
	std::cout << "Connected client socket to server\n";
}

void		Epoll::handleClose(t_serverData &server, t_clients &client)
{
	std::cout << "closing connection for client " << client._fd << " \n";
	auto it = std::find_if(server._clients.begin(), server._clients.end(), [&client](const t_clients &c) {return c._fd == client._fd; });

	if (it != server._clients.end())
		server._clients.erase(it);
	modifyEvent(client._fd, getEpfd(), EPOLL_CTL_DEL);
	protectedClose(client._fd);
}

void	Epoll::handleFile()
{
	// add file to epoll (error page)
	
}

void		Epoll::handleRead(t_serverData &server, t_clients &client)
{
	char			buffer[READ_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	ssize_t 		bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
	std::string		request; // http request

	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			std::cout << "no data available right now\n";	
			return ;
		}
		// recv failed
		handleClose(server, client);
		throw std::runtime_error("Reading from client connection failed\n");
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected\n";
		handleClose(server, client);
		return ;
	}
	
	// read protocol

	buffer[bytesRead] = '\0';
	request += buffer;
	std::cout << "Server received " << request << " from client " << client._fd << " \n";
	// http request handling here
	modifyEvent(client._fd, getEpfd(), EPOLLOUT);
}

void		Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	const char	response[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	//std::string	response1 = generateHttpResponse("this message from write");
	size_t		write_offset = 0; // keeping track of where we are in buffer
	
	ssize_t bytesWritten = send(client._fd, response + write_offset, strlen(response) - write_offset, 0);

	if (bytesWritten == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ; // no space in socket's send buffer, wait for more space
		std::cerr << "Write to client failed\n";
		handleClose(server, client);
		return ;
	}
	
	// write protocol
	
	write_offset += bytesWritten;
	// if all data sent, stop watching for write events (oui?)
	if (write_offset == strlen(response))
	{
		// reset buffer or process next message
		write_offset = 0;
		std::cout << "Client " << client._fd << " sent message to server: " << response << "\n\n\n";
	}
	modifyEvent(client._fd, getEpfd(), EPOLLIN);
}

void Epoll::makeNewConnection(int fd, t_serverData &server)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	int clientfd;

	clientfd = accept(fd, (struct sockaddr *)&clientAddr, &addrLen);
	if (clientfd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return;
	}
	else
	{
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen);
		addToEpoll(clientfd);
		server._clientTime[fd] = std::chrono::steady_clock::now();
		//server.setClientState(clientState::PARSING);
	}
}

void	Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		// not accessing the right varibales here!
		// std::cout << "looping servers " << &_serverData << " \n";
		// std::cout << "looping servers-clients " << &serverData._clients << " \n";
		//serverData._server->printServer();
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			std::cout << "handling new connection for server socket\n";
			makeNewConnection(fd, serverData);
			std::cout << "Amount of clients: " << serverData._clients.size() << std::endl;
			break ;
		}
		for (auto &client : serverData._clients)
		{
			//std::cout << "client fds = " << client._fd << " \n";
			if (fd == client._fd)
			{
				std::cout << "handling client socket with fd " << fd << " \n";
				if (event.events & EPOLLIN)
					handleRead(serverData, client);
				else if (event.events & EPOLLOUT)
					handleWrite(serverData, client);
				else if (event.events & EPOLLHUP)
					std::cout << "hup\n"; // will handle close here
			}
		}
	}
}

/* serverData methods */
void	s_serverData::addClient(int sock, struct sockaddr_in &addr, int len)
{
	t_clients	newClient;

	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;

	_clients.push_back(newClient);
}

void	s_serverData::setClientState(enum clientState state)
{
	this->_clientState = state;
}

enum clientState		s_serverData::getClientState()
{
	return this->_clientState;
}


/* getters */
int							Epoll::getEpfd() const
{
	return this->_epfd;
}

std::vector<t_serverData>	&Epoll::getAllServers()
{
	return this->_serverData;
}

std::shared_ptr<Server>		Epoll::getServer(size_t i)
{
	return this->_serverData[i]._server;
}

int							Epoll::getNumEvents() const
{
	return this->_numEvents;
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
void				Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void				Epoll::setServer(std::shared_ptr<Server> server)
{
	t_serverData	newServerData;
	newServerData._server = server;
	
	this->_serverData.push_back(newServerData);
}

void				Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}

void				Epoll::setEventMax()
{
	_events.resize(MAX_EVENTS);
}

void				Epoll::setEvent(struct epoll_event &event)
{
	this->_event = event;
}
