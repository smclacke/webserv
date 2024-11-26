/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/26 14:56:34 by smclacke      ########   odam.nl         */
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


/** @todo fix this */
void		Epoll::clientTime(t_serverData server)
{
	auto now = std::chrono::steady_clock::now();
	(void) now;
	(void) server;
	//for (auto it = server._clientTime.begin(); it != server._clientTime.end();)
	//{
	//	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
	//	if (elapsed.count() >= (TIMEOUT / 1000))
	//	{
	//		std::cout << "Client timed out\n";
	//		protectedClose(it->first);
	//		// remove server from epoll
	//		// delete client(it->first)
	//		// it = _clientTime.erase(it);
	//	}
	//	else
	//		it++;
	//}
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
	modifyEvent(client._fd, EPOLL_CTL_DEL);
	protectedClose(client._fd);
}

/** @todo make this function */
void	Epoll::handleFile()
{
	// add file to epoll (error page)
	
}

/* EXAMPLE */

	//	// read from the connection
	//	_bufferSize = sizeof(_buffer);
	//	while ((_bytesRead = read(_connection, _buffer, _bufferSize - 1)) > 0)
	//	{
	//		_buffer[_bytesRead] = '\0';
	//		_request += _buffer;
	//		if (_request.find("\r\n\r\n") != std::string::npos)
	//			break ; // end of HTTP request
	//	}
		
	//	if (_bytesRead < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	//		std::cerr << "error reading from connection\n";

	//	if (!_request.empty())
	//	{
	//		std::cout << "received request: " << _request << "\n";

	//		// send HTTP response
	//		std::string	_response = generateHttpResponse(std::string("get me on your browser"));
	//		if (send(_connection, _response.c_str(), _response.size(), 0) < 0)
	//			std::cerr << "error snding response to client\n";
	//	}
	//	else
	//		std::cout << "received empty request\n";

	//	close(_connection);
	//}

/* EXMAPLE TWO */

//char		buffer[1024];
//				int			bytesRead = 0;
//				std::string	request;
				
//				while (read(events[i].data.fd, buffer, sizeof(buffer) - 1) < 0)
//				{
//					buffer[bytesRead] = '\0';
//					request += buffer;
//					if (request.find("\r\n\r\n") != std::string::npos)
//						break ; // end of HTTP request
//				}
//				if (bytesRead <= 0)
//				{
//					close (events[i].data.fd);
//					std::cout << "Client disconnected\n";
//				}
//				if (!request.empty())
//				{
//					std::cout << "Server received request: " << request << "\n";
					
//					// send HTTP response
//					std::string	response = generateHttpResponse("here, have a response");

//					if (send(events[i].data.fd, response.c_str(), response.size(), 0) < 0)
//						std::cerr << "Error sending response to client\n";
//				}
//				else
//					std::cout << "received empty request\n";
//			}

/** @todo add bool for close or keep connection alive depneding on the header */
void		Epoll::handleRead(t_serverData &server, t_clients &client)
{
	(void) server; // dont need server (90% sure)
	// how to work with strstream request?
	
	if (client._clientState == clientState::BEGIN)
	{
		memset(client._buffer, 0, sizeof(client._buffer));
	}

	// read protocol
	if (client._clientState != clientState::READY)
	{
		while (client._bytesRead < READ_BUFFER_SIZE)
		{
			client._bytesRead = recv(client._fd, client._buffer, sizeof(client._buffer) - 1, 0);
			//client._clientState == clientState::READING;
			if (client._bytesRead < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					std::cout << "no data available right now\n";
					// client state
					// connection bool
					return ;
				}
				// recv failed
				handleClose(server, client);
				// connection bool ?
				throw std::runtime_error("Reading from client connection failed\n");
			}
			else if (client._bytesRead == 0)
			{
				std::cout << "Client disconnected\n";
				handleClose(server, client);
				// connection bool?
				return ;
			}
			else if (client._bytesRead == strlen(client._buffer))
			{
				client._buffer[client._bytesRead] = '\0';
				//client._request += client._buffer; // do this differently
				std::cout << "Server received " << client._buffer << " from client " << client._fd << " \n";
				client._connectionClose = false; // oui?
				client._clientState = clientState::READY;
				return ;
			}
			client._connectionClose = false; // here or in if?
		}
	}
}

/** @todo add bool for close or keep connection alive depneding on the header
 * @todo ensure generaterequest is not being called everytime for the same client when just filling buffer
 */
void		Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	(void) server; // dont need server (90% sure)
	if (client._response.empty()) // || if client state begin/ready, then generate
	{
		// if not started writing response, generate 
		//const char	response[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
		//std::string	response1 = generateHttpResponse("this message from write");
	
		//client._response = generateHttpResponse("this message from handleWrite()");
		client._response = "HTTP/1.1 200 OK\r\nContent-Length: 35\r\n\r\nHello, World!1234567890123456789012";
	}

	// write protocol
	if (client._clientState != clientState::READY)
	{
		while (client._bytesWritten < WRITE_BUFFER_SIZE)
		{
			client._bytesWritten = send(client._fd, client._response.c_str() + client._write_offset, strlen(client._response.c_str()) - client._write_offset, 0);
			if (client._bytesWritten == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					client._clientState = clientState::WRITING;
					client._connectionClose = false;
					return ; // no space in socket's send buffer, wait for more space
				}
				std::cerr << "Write to client failed\n";
				//handleClose(server, client);
				client._connectionClose = true;
				client._clientState = clientState::ERROR;
				return ;
			}
			
			client._clientState = clientState::WRITING;
			client._write_offset += client._bytesWritten;
			if (client._write_offset == strlen(client._response.c_str()))
			{
				client._write_offset = 0;
				std::cout << "Client " << client._fd << " sent message to server: " << client._response << "\n\n\n";
				client._connectionClose = false; // true? we done
				client._clientState = clientState::READY;
				return ;
			}
			client._connectionClose = false; // here or in if?
		}
	}
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
		
		/** @todo how do i handle client state + time correctly */
		//server._clients->_clientTime[fd] = std::chrono::steady_clock::now();
		//server.setClientState(clientState::BEGIN);
	}
}

/** @todo make sure we are not waiting for clients to finish read/write, continue after buffer done
 * 		once complete, then change EPOLL status to in/out and continue finishing that client
 * @todo finish handling HUP
 */
void	Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			std::cout << "handling new connection for server socket\n";
			makeNewConnection(fd, serverData);
			std::cout << "Amount of clients: " << serverData._clients.size() << std::endl;
			break ;
		}
		for (auto &client : serverData._clients)
		{
			if (fd == client._fd)
			{
				std::cout << "handling client socket with fd " << fd << " \n";
				if (event.events & EPOLLIN)
				{
					handleRead(serverData, client);
					if (client._clientState == clientState::READY)
					{
						std::cout << "client has finished reading\n";
						modifyEvent(client._fd, EPOLLOUT);
					}
				}
				else if (event.events & EPOLLOUT)
				{
					handleWrite(serverData, client);
					if (client._clientState == clientState::READY)
					{
						std::cout << "client has finished writing\n";
						modifyEvent(client._fd, EPOLLIN);
					}
				}
				else if (event.events & EPOLLHUP)
					std::cout << "hup\n"; // will handle close here
			}
		}
	}
}

/* clients methods */


/* serverData methods */
void	s_serverData::addClient(int sock, struct sockaddr_in &addr, int len)
{
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

	_clients.push_back(newClient);
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
