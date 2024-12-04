/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/04 21:20:18 by smclacke      ########   odam.nl         */
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

	if (n < 0)
	{
		std::cerr << "Read() failed\n";
		return ;	
	}
	if (n > 0)
	{
		buffer[n] = '\0';
		std::cout << "file read = " << buffer << "\n";
	}
}

void		Epoll::handleRead(t_clients &client)
{
	char		buffer[READ_BUFFER_SIZE];
	size_t		bytesRead = 0;

	client._clientState = clientState::READING;
	memset(buffer, 0, sizeof(buffer));
	bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
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
	buffer[READ_BUFFER_SIZE - 1] = '\0';
	std::string buf = buffer;
	std::cout << "recv BUFF ON EACH READ = " << buf << "\n";
	client._requestClient.append(buf);
	if (client._requestClient.find("\r\n\r\n") != std::string::npos)
	{
		std::cout << "recv = " << client._requestClient << "\n";
		client._clientState = clientState::READY;
		return ;
	}
	client._connectionClose = false;
}

/** 
 * @todo get actual response message || error page
 */
void		Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	// get the http response + send that | cleint.httpSend.msg
	if (client._responseClient.msg.empty() && client._readingFile == false)
	{
		client._clientState = clientState::WRITING;
		client._responseClient = server._server->handleRequest(client._requestClient);
		std::cout << "response = " << client._responseClient.msg << std::endl;
		std::cout << "readFD =" << client._responseClient.readFd << std::endl;
		client._requestClient.clear();		
	}
		//client._responseClient = "HTTP/1.1 200 OK\r\nContent-Length: 35\r\n\r\nHello, World!1234567890123456789012";

	if (client._readingFile == false)
	{
		ssize_t leftover;
		ssize_t sendlen = WRITE_BUFFER_SIZE;
		leftover = client._responseClient.msg.size() - client._write_offset;
		if (leftover < WRITE_BUFFER_SIZE)
		{
			sendlen = leftover;
		}
		ssize_t	bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, leftover, 0);
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
		if (client._write_offset >= client._responseClient.msg.length())
		{
			std::cout << "finished sending message\n";
			if (client._responseClient.readFd != -1)
				client._readingFile = true;
			else
			{
				client._clientState = clientState::READY;
				if (client._responseClient.keepAlive == false)
					client._connectionClose = true;	
			}
			client._write_offset = 0;
			client._responseClient.msg.clear();
			return ;
		}
		client._connectionClose = false;
	}
	else
	{
		std::cout << "WE ARE READING\n";
		//addToEpoll(client._responseClient.readFd);
		char buffer[READ_BUFFER_SIZE];
		ssize_t	bytesWritten = read(client._responseClient.readFd, buffer ,READ_BUFFER_SIZE);
		std::cout << "BYTES WRITTEN IS " << bytesWritten << std::endl;
		if (bytesWritten < 0) // error
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			std::cerr << "Reading from pipe failed\n";
			client._connectionClose = true;
			return ;
		}
		else if (bytesWritten == 0) // nothing to read anymore -> we are done
		{
			send(client._fd, "\r\n\r\n", 4, 0);
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
		else if (bytesWritten == READ_BUFFER_SIZE) // we are not done
		{
			std::cout << "WE READ:--" << buffer << std::endl;
			bytesWritten = send(client._fd, buffer, bytesWritten, 0);
			if (bytesWritten < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					return ;
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return ;
			}
		}
		else if (bytesWritten < READ_BUFFER_SIZE) // we need to send and then we are done
		{
			bytesWritten = send(client._fd, buffer, bytesWritten, 0);
			if (bytesWritten < 0)
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
			send(client._fd, "\r\n\r\n", 4, 0);
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;	
			return ;
		}
					
	}
	
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
 * @todo file stuff (??)
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
		for (auto &client : serverData._clients)
		{
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
				else if (event.events & EPOLLOUT)
				{
					handleWrite(serverData, client);
					if (client._clientState == clientState::READY)
					{	
						modifyEvent(client._fd, EPOLLIN);
						updateClientClock(client);
					}
				}
				else if (event.events & EPOLLHUP)
				{
					std::cout << "Epoll: EPOLLHUP\n";
					client._connectionClose = true;
				}
				else if (event.events & EPOLLRDHUP)
				{
					std::cout << "Epoll: EPOLLRDHUP\n";
					client._connectionClose = true;
				}
				else if (event.events & EPOLLERR)
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
