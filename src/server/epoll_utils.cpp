/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/20 16:28:39 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

/* Epoll utils */
void Epoll::addINEpoll(int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		protectedClose(fd);
		return;
	}
}

void Epoll::addOUTEpoll(int fd)
{
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		protectedClose(fd);
		return;
	}
}

void Epoll::modifyInANDOut(int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
		closeDelete(fd);
}

void Epoll::modifyEvent(int fd, uint32_t events)
{
	struct epoll_event event;

	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
		closeDelete(fd);
}

void Epoll::setNonBlocking(int connection)
{
	int flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void Epoll::updateClientClock(t_clients &client)
{
	client._clientTime[client._fd] = std::chrono::steady_clock::now();
}

void Epoll::clientTimeCheck(t_clients &client)
{
	auto now = std::chrono::steady_clock::now();

	for (auto it = client._clientTime.begin(); it != client._clientTime.end();)
	{
		int client_fd = it->first;
		auto lastActivity = it->second;

		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastActivity).count() > TIMEOUT)
		{
			closeDelete(client_fd);
			it = client._clientTime.erase(it);
		}
		else
			it++;
	}
}

void Epoll::closeDelete(int fd)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
	protectedClose(fd);
}

void Epoll::removeCGIFromEpoll(t_clients &client)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiIN[1], nullptr);
	epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiOUT[0], nullptr);
}

/**
 * @brief Removes CGI file descriptors from the epoll instance and cleans up the CGI state for the client.
 */
void Epoll::removeCgiFromClient(t_clients &client)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiIN[1], nullptr);
	protectedClose(client.cgi.cgiIN[1]);
	epoll_ctl(_epfd, EPOLL_CTL_DEL, client.cgi.cgiOUT[0], nullptr);
	protectedClose(client.cgi.cgiOUT[0]);
	client.cgi.clearCgi();
	client._responseClient.cgi = false;
}

void Epoll::handleClientClose(t_serverData &server, t_clients &client)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, client._fd, nullptr);
	protectedClose(client._fd);
	server.removeClient(client);
	server._server->logMessage("handleClientClose(): Removed a client from Epoll, server: " + server._server->getServerName());
}

void Epoll::operationFailed(t_clients &client)
{
	client.cgi.clearCgi();
	client._readingFile = false;
	client._responseClient.readfile = false;
	client._clientState = clientState::ERROR;
	client._connectionClose = true;
}

/**
 * @brief cleans the _responseclient s_httpsend and writeoffest and readingFile
 */
void Epoll::cleanResponse(t_clients &client)
{
	client._responseClient.msg.clear();
	if (client._responseClient.readfile)
		protectedClose(client._responseClient.readFd);

	client._responseClient.readfile = false;
	client._responseClient.cgi = false;

	client._write_offset = 0;
	client._readingFile = false;
	client.cgi.clearCgi();
}

void Epoll::clientCgiTimeCheck(t_clients &client)
{
	if (client.cgi.pid != -1)
	{
		time_t current_time;
		time(&current_time);
		if ((current_time - client.cgi.start) >= CGI_TIMEOUT)
		{
			std::cout << "timeout occurred\n";
			protectedChildProcessEnd(client.cgi.pid);
			removeCgiFromClient(client);
			client._responseClient.clearHttpSend();
			client._responseClient.msg = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\ncgi script timeout";
			client._clientState = clientState::BEGIN;
			modifyEvent(client._fd, EPOLLOUT);
		}
	}
}