/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/13 13:43:02 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

/* constructors */
Epoll::Epoll() : _epfd(0), _numEvents(MAX_EVENTS)
{
	setEventMax();
}

Epoll::~Epoll()
{
	if (_epfd > 0)
		protectedClose(_epfd);
}

/* methods */
void Epoll::initEpoll()
{
	_epfd = epoll_create(10);
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
}

void Epoll::handleRead(t_clients &client)
{
	client._clientState = clientState::READING;
	size_t readSize = client.http->getReadSize();
	char buffer[readSize];
	int bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));

	bytesRead = recv(client._fd, buffer, readSize - 1, 0);
	// Error
	if (bytesRead < 0)
	{
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		std::cerr << "handleRead(): recv() failed" << std::endl;
		return;
	}
	// Disconnected
	else if (bytesRead == 0)
	{
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return;
	}
	// Finished
	buffer[readSize - 1] = '\0';
	std::string buf = buffer;
	client.http->addStringBuffer(buf);
	if (client.http->getKeepReading())
		return;
	else
	{
		client._clientState = clientState::READY;
		client._connectionClose = false;
	}
}

void Epoll::handleWrite(t_clients &client)
{
	if (client._clientState == clientState::BEGIN)
	{
		client._clientState = clientState::WRITING;
	}
	if (client._readingFile)
	{
		handleFile(client);
		return;
	}
	ssize_t leftover;
	ssize_t sendlen = WRITE_BUFFER_SIZE;
	leftover = client._responseClient.msg.size() - client._write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;
	int bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, sendlen, 0);
	// Error
	if (bytesWritten < 0)
	{
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		std::cerr << "handleWrite: send() to client failed\n";
		return;
	}
	// Disconnected
	else if (bytesWritten == 0)
	{
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return;
	}
	client._write_offset += bytesWritten;
	// Finished
	if (client._write_offset >= client._responseClient.msg.length())
	{
		client._write_offset = 0;
		client._responseClient.msg.clear();
		if (client._responseClient.readfile)
		{
			client._readingFile = true;
			client._connectionClose = false;
			return;
		}
		else
		{
			client._clientState = clientState::READY;
			if (client._responseClient.keepAlive == false)
			{
				client._clientState = clientState::CLOSE;
				client._connectionClose = true;
			}
		}
		if (client._responseClient.keepAlive)
			client._connectionClose = false;
		else
			client._connectionClose = true;
		return;
	}
	client._connectionClose = false;
}

void Epoll::handleFile(t_clients &client)
{
	char buffer[READ_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	int bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE);
	if (bytesRead < 0)
	{
		std::cerr << "handleFile(): Reading from file failed\n";
		operationFailed(client);
		return;
	}
	else if (bytesRead == 0)
	{
		client._clientState = clientState::READY;
		if (!client._responseClient.keepAlive)
		{
			client._connectionClose = true;
		}
		return;
	}
	int bytesSent = send(client._fd, buffer, bytesRead, 0);
	if (bytesSent < 0)
	{
		std::cerr << "handleFile(): write to client failed\n";
		operationFailed(client);
		return;
	}
	client._clientState = clientState::WRITING;
}

void Epoll::makeNewConnection(int fd, t_serverData &server)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	if (server._clients.size() >= 10)
	{
		server._server.get()->logMessage("makeNewConnection(): too many clients, denying connection");
		return;
	}
	int clientfd = accept(fd, (struct sockaddr *)&clientAddr, &addrLen);
	if (clientfd < 0)
	{
		std::cerr << "Error accepting new connection\n";
		return;
	}
	else
	{
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen, *this);
		addINEpoll(clientfd);
		server._server->logMessage("makeNewConnection(): accepted new client connection, server: " + server._server->getServerName() + ", fd: " + std::to_string(fd));
	}
}

void Epoll::checkForNewConnection(int fd, t_serverData &serverData, epoll_event &event)
{
	for (const auto &client : serverData._clients)
	{
		if (fd == client._fd)
			return;
	}
	if (fd == serverData._server->getServerSocket()->getSockfd() && (event.events & EPOLLIN))
	{
		makeNewConnection(fd, serverData);
	}
}

void Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		checkForNewConnection(fd, serverData, event);
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
						client._responseClient = client.http->generateResponse();
						if (client._responseClient.cgi)
						{
							client.cgi = client.http->getCGI();
							client.cgi.client_fd = client._fd;
							client._responseClient.clearHttpSend();
						}
						else
							modifyEvent(client._fd, EPOLLOUT);
						client.http->clearHandler();
						updateClientClock(client);
					}
				}
				if (event.events & EPOLLOUT)
				{
					handleWrite(client);
					if (client._clientState == clientState::READY)
					{
						cleanResponse(client);
						client._clientState = clientState::BEGIN;
						modifyEvent(client._fd, EPOLLIN);
						updateClientClock(client);
					}
				}
				if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
				{
					client._connectionClose = true;
					cleanResponse(client);
					client.http->clearHandler();
				}
				if (client._connectionClose || client._clientState == clientState::ERROR)
					handleClientClose(serverData, client);
			}
			else if (fd == client.cgi.cgiIN[1] || fd == client.cgi.cgiOUT[0])
				cgiEvent(fd, client, event);
		}
	}
}

void Epoll::cgiEvent(int &fd, t_clients &client, epoll_event &event)
{
	if (fd == client.cgi.cgiIN[1])
	{
		std::cout << "CGI IN event triggerd" << std::endl; /** @todo remove after testing*/
		if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
		{
			std::cout << "CGI-IN EPOLLHUP event triggerd" << std::endl; /** @todo remove after testing*/
			epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiIN[1], nullptr);
			protectedClose(client.cgi.cgiIN[1]);
		}
		if (event.events & EPOLLOUT && fd == client.cgi.cgiIN[1])
		{
			std::cout << "CGI-IN EPOLLOUT event triggerd" << std::endl; /** @todo remove after testing*/
			handleCgiWrite(client.cgi);
			if (client.cgi.state == cgiState::ERROR)
			{
				removeCGIFromEpoll(client);
				modifyEvent(client.cgi.client_fd, EPOLLOUT);
				int status;
				if (client.cgi.pid != -1)
				{
					waitpid(client.cgi.pid, &status, 0);
				}
				client.cgi.output = internalError("cgi error");
				client._responseClient.clearHttpSend();
				if (client.cgi.httpOutput)
				{
					client._responseClient.msg = "HTTP/1.1 500 Internal server error\r\n";
					client._responseClient.msg.append("Content-Length: 0\r\n");
					client._responseClient.msg.append("\r\n");
				}
				else
					client._responseClient.msg = client.cgi.output;
				client.cgi.clearCgi();
			}
			if (client.cgi.state == cgiState::READY)
			{
				epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiIN[1], nullptr);
				protectedClose(client.cgi.cgiIN[1]);
			}
		}
	}
	if (fd == client.cgi.cgiOUT[0])
	{
		if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
		{
			std::cout << "CGI-OUT EPOLLHUP event triggerd" << std::endl; /** @todo remove after testing*/
			epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiOUT[0], nullptr);
			protectedClose(client.cgi.cgiOUT[0]);
			modifyEvent(client.cgi.client_fd, EPOLLOUT);
			int status;
			if (client.cgi.pid != -1)
			{
				waitpid(client.cgi.pid, &status, 0);
				if (WEXITSTATUS(status) != 0)
					client.cgi.output = internalError("cgi bad exitstatus");
				if (client.cgi.output.empty())
					client.cgi.output = internalError("cgi executed");
			}
			client._responseClient.clearHttpSend();
			if (client.cgi.httpOutput)
			{
				client._responseClient.msg = "HTTP/1.1 200 OK\r\n";
				client._responseClient.msg.append("Content-Length: " + std::to_string(client.cgi.output.size()) + "\r\n");
				client._responseClient.msg.append("\r\n");
			}
			client._responseClient.msg.append(client.cgi.output);
			client.cgi.clearCgi();
		}
		if (event.events & EPOLLIN && fd == client.cgi.cgiOUT[0])
		{
			std::cout << "CGI-OUT EPOLLIN event triggerd" << std::endl; /** @todo remove after testing*/
			handleCgiRead(client.cgi);
			if (client.cgi.state == cgiState::ERROR)
			{
				removeCGIFromEpoll(client);
				protectedClose(client.cgi.cgiOUT[0]);
				modifyEvent(client.cgi.client_fd, EPOLLOUT);
				int status;
				waitpid(client.cgi.pid, &status, 0);
				client.cgi.output = internalError("cgi error");
				client._responseClient.clearHttpSend();
				if (client.cgi.httpOutput)
				{
					client._responseClient.msg = "HTTP/1.1 500 Internal server error\r\n";
					client._responseClient.msg.append("Content-Length: 0\r\n");
					client._responseClient.msg.append("\r\n");
				}
				client._responseClient.msg = client.cgi.output;
				client.cgi.clearCgi();
			}
			if (client.cgi.state == cgiState::READY)
			{
				epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiOUT[0], nullptr);
				protectedClose(client.cgi.cgiOUT[0]);
				modifyEvent(client.cgi.client_fd, EPOLLOUT);
				int status;
				if (client.cgi.pid != -1)
				{
					waitpid(client.cgi.pid, &status, 0);
					if (WEXITSTATUS(status) != 0)
						client.cgi.output = internalError("cgi bad exitstatus");
				}
				client._responseClient.clearHttpSend();
				if (client.cgi.httpOutput)
				{
					client._responseClient.msg = "HTTP/1.1 200 OK\r\n";
					client._responseClient.msg.append("Content-Length: " + std::to_string(client.cgi.output.size()) + "\r\n");
					client._responseClient.msg.append("\r\n");
				}
				client._responseClient.msg.append(client.cgi.output);
				client.cgi.clearCgi();
			}
		}
	}
}

/* getters */
int Epoll::getEpfd() const
{
	return this->_epfd;
}

int Epoll::getNumEvents() const
{
	return this->_numEvents;
}

std::vector<t_serverData> &Epoll::getAllServers()
{
	return this->_serverData;
}

std::shared_ptr<Server> Epoll::getServer(size_t i)
{
	return this->_serverData[i]._server;
}

std::vector<epoll_event> &Epoll::getAllEvents()
{
	return this->_events;
}

struct epoll_event &Epoll::getEvent()
{
	return this->_event;
}

/* setters */
void Epoll::setEpfd(int fd)
{
	this->_epfd = fd;
}

void Epoll::setNumEvents(int numEvents)
{
	this->_numEvents = numEvents;
}

void Epoll::setEventMax()
{
	this->_events.resize(MAX_EVENTS);
}

void Epoll::setEvent(struct epoll_event &event)
{
	this->_event = event;
}

void Epoll::setServer(std::shared_ptr<Server> server)
{
	t_serverData newServerData;
	newServerData._server = server;

	this->_serverData.push_back(std::move(newServerData));
}
