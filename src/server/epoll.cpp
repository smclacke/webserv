/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/29 13:33:32 by smclacke      ########   odam.nl         */
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

/** @todo check everything that needs to be cleaned here + freeaddress and event stuff */
Epoll::~Epoll() 
{ 
	std::cout << "epoll destructor called\n";
	if (_epfd > 0)
	{
		if (!protectedClose(_epfd))
			std::cerr << "Failed to close epfd\n";
	}
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

/** @todo add bool for close or keep connection alive depneding on the header
 *  @todo handling bad fd to recv, not necessarily a throw
 */
void		Epoll::handleRead(t_serverData &server, t_clients &client)
{
	char		buffer[READ_BUFFER_SIZE];
	size_t		bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));

	// read protocol
	while (bytesRead < READ_BUFFER_SIZE)
	{
		bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
		client._clientState = clientState::READING;
		if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cout << "no data available right now\n";
				client._clientState = clientState::BEGIN;
				return ;
			}
			handleClose(server, client);
			throw std::runtime_error("Reading from client connection failed\n");
		}
		else if (bytesRead == 0)
		{
				std::cout << "Client disconnected\n";
				handleClose(server, client);
				return ;
		}
		client._request += buffer;

		/** @todo  depends on our protocol for handling read, if finished or till max etc... */
		if (client._request.find("\r\n\r\n") != std::string::npos)
		{
			buffer[bytesRead] = '\0';
			client._request += buffer;
			std::cout << "request = " << client._request << "\n";

			client._clientState = clientState::READY;
			return ;
		}
		client._connectionClose = false;
	}
}

/** @todo add bool for close or keep connection alive depneding on the header
 * @todo ensure generaterequest is not being called everytime for the same client when just filling buffer
 */
void		Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	if (client._response.empty())
		client._response = "HTTP/1.1 200 OK\r\nContent-Length: 35\r\n\r\nHello, World!1234567890123456789012";

	// write protocol
	while (client._bytesWritten < WRITE_BUFFER_SIZE)
	{
		client._bytesWritten = send(client._fd, client._response.c_str() + client._write_offset, strlen(client._response.c_str()) - client._write_offset, 0);
		client._clientState = clientState::WRITING;
		if (client._bytesWritten == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ; // no space in socket's send buffer, wait for more space

			std::cerr << "Write to client failed\n";
			handleClose(server, client);
			return ;
		}
		client._write_offset += client._bytesWritten;
		if (client._write_offset == strlen(client._response.c_str()))
		{
			client._write_offset = 0;
			std::cout << "Client " << client._fd << " sent message to server: " << client._response << "\n\n\n";
			client._clientState = clientState::READY;
			return ;
		}
		client._connectionClose = false;
	}
}

/** @todo finish initialisation */
void	s_serverData::addClient(int sock, struct sockaddr_in &addr, int len)
{
	static int clientId = 0;

	t_clients	newClient;

	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;
	newClient._clientState = clientState::BEGIN;
	// client time... init here?
	newClient._connectionClose = false;
	// request/response here needed?
	newClient._write_offset = 0;
	newClient._bytesWritten = 0;
	newClient._clientId = clientId++;
	
	_clients.push_back(newClient);

	std::cout << "new client connected with ID: " << newClient._clientId << " from " << inet_ntoa(newClient._addr.sin_addr) << "\n";
}

/** @todo this: CONNECT (?) - 
 *		When Do You Use connect()?
		Client side: If your server is also acting as a client (for example, connecting to an external service), you would use connect() in that case to connect to another server.
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
		return;
	}
	else
	{
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen);
		addToEpoll(clientfd);
		
		/** @todo how do i handle client state + time correctly */
		//server._clients->_clientTime[fd] = std::chrono::steady_clock::now();
		//server.setClientState(clientState::BEGIN);
	}
}

/** @todo make sure we are not waiting for clients to finish read/write, continue after buffer done
 * 		once complete, then change EPOLL status to in/out and continue finishing that client
 * @todo finish handling HUP
 * @todo i need to know when/where/how im getting connectionclose 1/0 and then i can add the bool correctly
 */
void	Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			if (event.events & EPOLLIN)
			{
				std::cout << "handling new connection for server socket\n";
				makeNewConnection(fd, serverData);
			}
		}
		if (fd == _pipefd[0])
			handleFile();
		for (auto &client : serverData._clients)
		{
			if (fd == client._fd)
			{
				if (event.events & EPOLLIN)
				{
					handleRead(serverData, client);
					if (client._clientState == clientState::READY)
						modifyEvent(client._fd, EPOLLOUT);
				}
				else if (event.events & EPOLLOUT)
				{
					handleWrite(serverData, client);
					if (client._clientState == clientState::READY)
						modifyEvent(client._fd, EPOLLIN);
				}
				else if (event.events & EPOLLHUP)
					std::cout << "hup\n"; // will handle close here
			}
		}
	}
	// clean up - file pipe etc closed
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
