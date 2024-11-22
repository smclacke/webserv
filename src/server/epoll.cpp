/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/22 15:27:19 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

/**
 * @todo destruction + clean up
 * @todo clientStatus delete
 * @todo make new connection function
 */

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

void		Epoll::connectClient(t_serverData server)
{
	if ((connect(server._clientSock, (struct sockaddr*)&server._serverAddr, server._serverAddlen)))
	{
		if (errno != EINPROGRESS)
		{
			protectedClose(server._clientSock);
			protectedClose(server._serverSock);
			protectedClose(_epfd);
			throw std::runtime_error("Failed to connect client socket to server\n");
		}
	}
	std::cout << "Connected client socket to server\n";
}

void		Epoll::handleRead(t_serverData server, int i)
{
	char			buffer[READ_BUFFER_SIZE];
	std::string		request; // http request

	ssize_t bytesRead = recv(server._events[i].data.fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ; // no data available right now
		
		// recv failed
		// epoll_ctl(EPOLL_CTL_DEL)
		protectedClose(server._events[i].data.fd);
		throw std::runtime_error("Reading from client socket failed\n");
	}
	else if (bytesRead == 0)
	{
		// epoll_ctl(EPOLL_CTL_DEL)
		protectedClose(server._events[i].data.fd);
		std::cout << "Client disconnected\n";
	}
	else
	{
		// read protocol
		// process_incoming_data() - process a request, store it, or send a response.
		buffer[bytesRead] = '\0';
		request += buffer;
		std::cout << "Server received " << request << "\n";
		switchOUTMode(server._events[i].data.fd, _epfd, server._event);
	}
}

void		Epoll::handleWrite(t_serverData server, int i)
{
	const char	response[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	std::string	response1 = generateHttpResponse("this message from write");
	size_t		write_offset = 0; // keeping track of where we are in buffer
	
	ssize_t bytesWritten = send(server._events[i].data.fd, response + write_offset, strlen(response) - write_offset, 0);

	if (bytesWritten == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ; // no space in scoket's send buffer, wait for more space
		std::cerr << "Write to client failed\n";
		// handle connection close
		return ;
	}
	else if (bytesWritten > 0)
	{
		// write protocol
		write_offset += bytesWritten;

		// if all data sent, stop watching for write events (oui?)
		if (write_offset == strlen(response))
		{
			// reset buffer or process next message
			write_offset = 0;
			std::cout << "Client sent message to server: " << response << "\n\n\n";
			switchINMode(server._events[i].data.fd, _epfd, server._event);
		}
	}
}

void	Epoll::makeNewConnection(std::shared_ptr<Socket> &serverSock, t_serverData server)
{
	(void) serverSock;
	int						newSock;
	struct sockaddr_in		clientAddr;
	socklen_t				addrLen = sizeof(clientAddr);

	newSock = accept(server._serverSock, (struct sockaddr *)&clientAddr, &addrLen);
	if (newSock < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return ;
	}
	else 
	{
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(newSock);
		addToEpoll(newSock, _epfd, server._event);
		server.addClient(newSock, clientAddr, addrLen);
		server._clientTime[newSock] = std::chrono::steady_clock::now();
		server.setClientState(clientState::PARSING);
	}
}

void	Epoll::handleFile()
{
	// add file to epoll (error page)
	
}

/* serverData methods */
void	s_serverData::addClient(int sock, struct sockaddr_in addr, int len)
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
int					Epoll::getEpfd() const
{
	return this->_epfd;
}

std::vector<t_serverData>	Epoll::getAllServers() const
{
	return this->_serverData;
}

t_serverData		Epoll::getServer(size_t i) const
{
	return this->_serverData[i];
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

void				Epoll::setServer(t_serverData server)
{
	this->_serverData.push_back(server);
}

void				Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}
