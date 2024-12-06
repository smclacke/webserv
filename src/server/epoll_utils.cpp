/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/06 14:07:21 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

/* Epoll utils */
void		Epoll::addToEpoll(int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		protectedClose(fd);
		std::cerr << "Error adding fd to epoll\n";
		return ;
	}
}

void		Epoll::modifyEvent(int fd, uint32_t events)
{
	struct epoll_event event;

	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		closeDelete(fd);
		std::cerr << "Failed to modify socket event type\n";
	}
}

void		Epoll::setNonBlocking(int connection)
{
	int flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void		Epoll::updateClientClock(t_clients &client)
{
	client._clientTime[client._fd] = std::chrono::steady_clock::now();
}

void		Epoll::clientTimeCheck(t_clients &client)
{
	auto	now = std::chrono::steady_clock::now();

	for (auto it = client._clientTime.begin(); it != client._clientTime.end();)
	{
		int client_fd = it->first;
		auto lastActivity = it->second;

		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastActivity).count() > TIMEOUT)
		{
			std::cerr << "Client timed out\n";
			closeDelete(client_fd);
			it = client._clientTime.erase(it);	
		}
		else
			it++;
	}
}

void		Epoll::closeDelete(int fd)
{
	if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr) < 0)
		std::cerr << "Failed to remove fd from epoll\n";
	protectedClose(fd);
}

void		Epoll::handleClientClose(t_serverData &server, t_clients &client)
{
	if (epoll_ctl(_epfd, EPOLL_CTL_DEL, client._fd, nullptr) == -1)
		std::cerr << "Failed to remove fd from epoll\n";

	protectedClose(client._fd);
    client._requestClient.clear();
    client._responseClient.msg.clear();
	client._fd = -1;

    client._clientState = clientState::CLOSED;
	server.removeClient(client);
}


/** @todo remove when handleWrite and handleFile all good */
void	Epoll::handleBigWrite(t_serverData &server, t_clients &client)
{
	if (client._responseClient.msg.empty() && client._readingFile == false)
	{
		client._responseClient = server._server->handleRequest(client._requestClient);
		client._clientState = clientState::WRITING;
		client._requestClient.clear();
	}
	
	// not handling file
	if (client._readingFile == false)
	{
		ssize_t leftover;
		ssize_t sendlen = WRITE_BUFFER_SIZE;
		leftover = client._responseClient.msg.size() - client._write_offset;
		if (leftover < WRITE_BUFFER_SIZE)
		{
			sendlen = leftover;
		}
		ssize_t bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, leftover, 0);
		if (bytesWritten < 0)
		{
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return;
		}
		else if (bytesWritten == 0)
		{
			//std::cout << "Client disconnected\n";
			client._connectionClose = true;
			return;
		}
		client._write_offset += bytesWritten;
		if (client._write_offset >= client._responseClient.msg.length())
		{
			if (client._responseClient.readFd != -1)
			{
				client._readingFile = true;
				client._connectionClose = false;
				return ;
			}
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
	else // handling file
	{
		// to handle file, need to add to epoll right?? this doesnt work... :)
		//addToEpoll(client._responseClient.readFd);
		
		ssize_t bytesSend;
		char buffer[READ_BUFFER_SIZE];
		ssize_t bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE - 1);
		if (bytesRead < 0) // error
		{
			std::cerr << "Reading from pipe failed\n";
			client._connectionClose = true;
			return;
		}
		else if (bytesRead == 0) // nothing to read anymore -> we are done
		{
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
			return;
		}
		buffer[READ_BUFFER_SIZE - 1] = '\0';
		if (bytesRead == READ_BUFFER_SIZE - 1) // we are not done
		{
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return;
			}
		}
		else if (bytesRead < READ_BUFFER_SIZE) // we need to send and then we are done
		{
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return;
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
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;
			return;
		}
	}
}
