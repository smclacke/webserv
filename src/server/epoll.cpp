/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/13 12:16:47 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

/** @todo check cgi stuff here */

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
		return;
	}

	// Disconnected
	else if (bytesRead == 0)
	{
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return;
	}
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
		std::cerr << "Write to client failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
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
		std::cerr << "Reading from file failed\n";
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
		std::cerr << "Write to client failed\n";
		operationFailed(client);
		return;
	}
	client._clientState = clientState::WRITING;
}

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
		setNonBlocking(clientfd);
		server.addClient(clientfd, clientAddr, addrLen, *this);
		addToEpoll(clientfd);
	}
}

void Epoll::processEvent(int fd, epoll_event &event)
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
						client._responseClient = client.http->generateResponse();
						if (client._responseClient.cgi)
						{
							serverData.cgi = client.http->getCGI();
							serverData.cgi.client_fd = client._fd;
							// modifyInANDOut(serverData.cgi.client_fd);
						}
						else
						{
							modifyEvent(client._fd, EPOLLOUT);
						}
						client.http->clearHandler();
					}
					updateClientClock(client);
				}
				if (event.events & EPOLLOUT)
				{
					if (client._responseClient.cgi == true)
					{
						if (serverData.cgi.complete)
						{
							s_httpSend newResposne = {serverData.cgi.output, true, false, -1, false};
							client._responseClient = newResposne;
						}
					}
					handleWrite(client);
					if (client._clientState == clientState::READY)
					{
						cleanResponse(client);
						client._clientState = clientState::BEGIN;
						modifyEvent(client._fd, EPOLLIN);
					}
					updateClientClock(client);
				}
				if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
				{
					client._connectionClose = true;
					cleanResponse(client);
					client.http->clearHandler();
				}
				if (client._connectionClose)
					handleClientClose(serverData, client);
			}
			if (fd == serverData.cgi.cgiIN[1] || fd == serverData.cgi.cgiOUT[0])
				cgiEvent(fd, serverData, event);
		}
	}
}

void Epoll::cgiEvent(int &fd, t_serverData &serverData, epoll_event &event)
{
	if (fd == serverData.cgi.cgiIN[1])
	{
		if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
		{
			epoll_ctl(_epfd, EPOLL_CTL_DEL, serverData.cgi.cgiIN[1], nullptr);
			protectedClose(serverData.cgi.cgiIN[1]);
		}
		if (event.events & EPOLLOUT && fd == serverData.cgi.cgiIN[1])
		{
			handleCgiWrite(serverData.cgi);
			if (serverData.cgi.state == cgiState::ERROR)
			{
				removeCGIFromEpoll(serverData);
				modifyEvent(serverData.cgi.client_fd, EPOLLOUT);
				int status;
				if (serverData.cgi.pid != -1)
				{
					waitpid(serverData.cgi.pid, &status, 0);
				}
				serverData.cgi.output = internalError("cgi error");
				serverData.cgi.complete = true;
			}
			if (serverData.cgi.state == cgiState::READY)
			{
				epoll_ctl(_epfd, EPOLL_CTL_DEL, serverData.cgi.cgiIN[1], nullptr);
				protectedClose(serverData.cgi.cgiIN[1]);
			}
		}
	}
	if (fd == serverData.cgi.cgiOUT[0])
	{
		if (event.events & EPOLLHUP || event.events & EPOLLRDHUP || event.events & EPOLLERR)
		{
			epoll_ctl(_epfd, EPOLL_CTL_DEL, serverData.cgi.cgiOUT[0], nullptr);
			protectedClose(serverData.cgi.cgiOUT[0]);
			modifyEvent(serverData.cgi.client_fd, EPOLLOUT);
			int status;
			if (serverData.cgi.pid != -1)
			{
				waitpid(serverData.cgi.pid, &status, 0);
				if (WEXITSTATUS(status) != 0)
					serverData.cgi.output = internalError("cgi bad exitstatus");
				if (serverData.cgi.output.empty())
					serverData.cgi.output = internalError("cgi executed");
			}
		}
		if (event.events & EPOLLIN && fd == serverData.cgi.cgiOUT[0])
		{
			handleCgiRead(serverData.cgi);
			if (serverData.cgi.state == cgiState::ERROR)
			{
				removeCGIFromEpoll(serverData);
				protectedClose(serverData.cgi.cgiOUT[0]);
				modifyEvent(serverData.cgi.client_fd, EPOLLOUT);
				int status;
				waitpid(serverData.cgi.pid, &status, 0);
				serverData.cgi.output = internalError("cgi error");
			}
			if (serverData.cgi.state == cgiState::READY)
			{
				epoll_ctl(_epfd, EPOLL_CTL_DEL, serverData.cgi.cgiOUT[0], nullptr);
				protectedClose(serverData.cgi.cgiOUT[0]);
				modifyEvent(serverData.cgi.client_fd, EPOLLOUT);
				int status;
				if (serverData.cgi.pid != -1)
				{
					waitpid(serverData.cgi.pid, &status, 0);
					if (WEXITSTATUS(status) != 0)
						serverData.cgi.output = internalError("cgi bad exitstatus");
				}
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
	return _events;
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
	_events.resize(MAX_EVENTS);
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