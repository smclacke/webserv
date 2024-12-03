/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/03 22:58:13 by smclacke      ########   odam.nl         */
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
	std::cout << "Epoll destructor called\n";
	if (_epfd > 0)
		protectedClose(_epfd);

	/** @todo if the pipe moves, move the close
	 * @todo if closeDelete fails? or pipe not added to epoll? */ 
	//if (_pipefd[0])
	//	closeDelete(_pipefd[0]);
	//if (_pipefd[1])
	//	closeDelete(_pipefd[1]);
}

/* methods */
void Epoll::initEpoll()
{
	_epfd = epoll_create(10);
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

void	Epoll::handleFile()
{
	char	buffer[MAX_FILE_READ];
	int		n = read(_pipefd[0], buffer, sizeof(buffer) - 1);

	if (n > 0)
	{
		buffer[n] = '\0';
		std::cout << "file read = " << buffer << "\n";
	}
}

/** 
 *  @todo handling bad fd to recv, not necessarily a throw or exit server situ
 */
void		Epoll::handleRead(t_clients &client)
{
	char		buffer[READ_BUFFER_SIZE];
	size_t		bytesRead = 0;

	while (true)
	{
		memset(buffer, 0, sizeof(buffer));

		bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
		client._clientState = clientState::READING;
		if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cout << "No data available right now\n";
				client._clientState = clientState::BEGIN;
				return ;
			}
			
			std::cerr << "Reading from client connection failed\n";
			client._connectionClose = true;
			return ;
		}
		else if (bytesRead == 0)
		{
			std::cout << "Client disconnected\n";
			client._connectionClose = true;
			return ;
		}
		client._request += buffer;
		if (client._request.find("\r\n\r\n") != std::string::npos)
		{
			std::cout << "request = " << client._request << "\n";
			client._clientState = clientState::READY;
			client._request.clear();
			return ;
		}
		client._connectionClose = false;
	}
}

/** 
 * @todo get actual response message || error page
 */
void		Epoll::handleWrite(t_clients &client)
{
	if (client._response.empty())
		client._response = "HTTP/1.1 200 OK\r\nContent-Length: 35\r\n\r\nHello, World!1234567890123456789012";

	while (true)
	{
		ssize_t	bytesWritten = send(client._fd, client._response.c_str() + client._write_offset, strlen(client._response.c_str()) - client._write_offset, 0);
		if (bytesWritten < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return ;
		}
		else if (bytesWritten == 0)
		{
			std::cout << "Client disconnected\n";
			client._connectionClose = true;
			return ;
		}
		client._write_offset += bytesWritten;
		if (client._write_offset >= client._response.length())
		{
			client._write_offset = 0;
			client._response.clear();
			client._clientState = clientState::READY;
			return ;
		}
		client._connectionClose = false;
	}
}

void	s_serverData::addClient(int sock, struct sockaddr_in &addr, int len)
{
	static int clientId = 0;

	t_clients	newClient;

	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;
	newClient._clientState = clientState::BEGIN;
	newClient._clientTime[sock] = std::chrono::steady_clock::now();
	newClient._connectionClose = false;
	newClient._write_offset = 0;
	newClient._bytesWritten = 0;
	newClient._clientId = clientId++;
	
	_clients.push_back(newClient);

	std::cout << "New client connected with ID: " << newClient._clientId << " from " << inet_ntoa(newClient._addr.sin_addr) << "\n";
}

/** @todo this: CONNECT (?) - this is not necessary right?
 *		When Do You Use connect()?
		Client side: If your server is also acting as a client (for example, connecting to an external service),
		you would use connect() in that case to connect to another server.
		Example: A client program connecting to a remote server (via connect()).
		Another example: A server acting as a proxy or a backend service connecting to a database.
 */
void Epoll::makeNewConnection(int fd, t_serverData &server)
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
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen);
		addToEpoll(clientfd);
	}
}

/** 
 * @todo file stuff
 */
void	Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			if (event.events & EPOLLIN)
				makeNewConnection(fd, serverData);
		}
		//std::string file = 0;
		//if (!file.empty())
		//	addFile();
		for (auto &client : serverData._clients)
		{
			//if (fd == _pipefd[0])
			//	handleFile();
			if (fd == client._fd)
			{
				if (event.events & EPOLLIN)
				{
					handleRead(client);
					if (client._clientState == clientState::READY)
					{
						modifyEvent(client._fd, EPOLLOUT);
						updateClientClock(client);
					}
				}
				if (event.events & EPOLLOUT)
				{
					handleWrite(client);
					if (client._clientState == clientState::READY)
					{	
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
void Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void				Epoll::setServer(std::shared_ptr<Server> server)
{
	t_serverData	newServerData;
	newServerData._server = server;
	
	this->_serverData.push_back(newServerData);
}

void Epoll::setNumEvents(int numEvents)
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
