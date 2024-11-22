/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/23 00:43:23 by juliusdebaa   ########   odam.nl         */
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

	// if (_serverfd)
	//	protectedClose(_serverfd);
	// if (_clientfd)
	//	protectedClose(_clientfd);
	//}
	std::cout << "epoll destructor called\n";
	if (_epfd)
		protectedClose(_epfd);

	// freeaddrinfo
	// more clean
}

/* methods */
void Epoll::initEpoll()
{
	_epfd = epoll_create(10);
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

void Epoll::clientTime(t_serverData server)
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
			protectedClose(server._server->getServerSocket()->getSockfd()); // change to server._server->serversocket->getfd();
			protectedClose(_epfd);
			throw std::runtime_error("Failed to connect client socket to server\n");
		}
	}
	std::cout << "Connected client socket to server\n";
}

void Epoll::handleRead(t_serverData server, int j)
{

	(void)server;
	char buffer[READ_BUFFER_SIZE];

	ssize_t bytesRead = recv(_events[j].data.fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // no data available right now

		// recv failed
		// epoll_ctl(EPOLL_CTL_DEL)
		protectedClose(_events[j].data.fd);
		throw std::runtime_error("Reading from client connection failed\n");
	}
	else if (bytesRead == 0)
	{
		// epoll_ctl(EPOLL_CTL_DEL)
		// protectedClose(server._events[j].data.fd);
		// std::cout << "Client disconnected\n";
		return; // should client connection be disconnected everytime??
	}
	else
	{
		// read protocol
		// process_incoming_data() - process a request, store it, or send a response.
		buffer[bytesRead] = '\0';
		_request << buffer; // Append to the _request stringstream
		std::cout << "Server received: " << _request.str() << "\n";
		switchOUTMode(_events[j].data.fd, _epfd, _event);
	}
}

void Epoll::handleWrite(t_serverData server, int j)
{
	_response = server._server->handleRequest(_request);
	_request.str(""); // clear epoll _request
	_request.clear(); // clear epoll _request errors
	// const char response[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	// std::string _response = generateHttpResponse("this message from write");
	size_t write_offset = 0; // keeping track of where we are in buffer
	ssize_t bytesWritten = send(_events[j].data.fd, _response.c_str() + write_offset, _response.size() - write_offset, 0);
	if (bytesWritten == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // no space in socket's send buffer, wait for more space
		std::cerr << "Write to client failed\n";
		// handle connection close
		return;
	}
	else if (bytesWritten > 0)
	{
		// write protocol
		write_offset += bytesWritten;

		// if all data sent, stop watching for write events
		if (write_offset == _response.size())
		{
			// reset buffer or process next message
			_response = ""; // Move this line here
			write_offset = 0;
			std::cout << "Client sent message to server: " << _response << "\n\n\n";
			switchINMode(_events[j].data.fd, _epfd, _event);
		}
	}
}

void Epoll::makeNewConnection(int fd, t_serverData server)
{
	struct sockaddr_in clientAddr; // check the addresses cause it doesnt make sense to me where they come from
	socklen_t addrLen = sizeof(clientAddr);

	fd = accept(server._server->getServerSocket()->getSockfd(), (struct sockaddr *)&clientAddr, &addrLen);
	if (fd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return;
	}
	else
	{
		std::cout << "\nNew connection made from " << inet_ntoa(clientAddr.sin_addr) << "\n";
		setNonBlocking(fd);
		addToEpoll(fd, _epfd, _event);
		server.addClient(fd, clientAddr, addrLen);
		server._clientTime[fd] = std::chrono::steady_clock::now();
		server.setClientState(clientState::PARSING);
	}
}

void Epoll::handleFile()
{
	// add file to epoll (error page)
}

/* serverData methods */
void s_serverData::addClient(int sock, struct sockaddr_in addr, int len)
{
	t_clients newClient;

	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;

	_clients.push_back(newClient);
}

void s_serverData::setClientState(enum clientState state)
{
	this->_clientState = state;
}

enum clientState s_serverData::getClientState()
{
	return this->_clientState;
}

/* getters */
int Epoll::getEpfd() const
{
	return this->_epfd;
}

std::vector<t_serverData> Epoll::getAllServers() const
{
	return this->_serverData;
}

t_serverData Epoll::getServer(size_t i) const
{
	return this->_serverData[i];
}

int Epoll::getNumEvents() const
{
	return this->_numEvents;
}

std::vector<epoll_event> &Epoll::getAllEvents()
{
	return _events;
}

struct epoll_event Epoll::getEvent()
{
	return this->_event;
}

/* setters */
void Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void Epoll::setServer(t_serverData server)
{
	this->_serverData.push_back(server);
}

void Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}

void Epoll::setEventMax()
{
	_events.resize(MAX_EVENTS);
}

void Epoll::setEvent(struct epoll_event event)
{
	this->_event = event;
}
