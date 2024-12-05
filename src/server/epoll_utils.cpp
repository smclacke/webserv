/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/05 17:37:27 by smclacke      ########   odam.nl         */
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
	std::cout << "New fd added to epoll: " << event.data.fd << "\n";
}

struct epoll_event Epoll::addServerSocketEpoll(int sockfd)
{
	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN;

	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		protectedClose(sockfd);
		std::cerr << "Error adding socket to epoll\n";
	}
	std::cout << "New server socket added to epoll: " << event.data.fd << "\n";
	return event;
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
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
	protectedClose(fd);
}

void		Epoll::handleClientClose(t_serverData &server, t_clients &client)
{
	if (epoll_ctl(_epfd, EPOLL_CTL_DEL, client._fd, nullptr) == -1)
		std::cerr << "Failed to remove fd from epoll\n";

	protectedClose(client._fd);
    client._clientState = clientState::CLOSED;
    client._requestClient.clear();
    client._responseClient.msg.clear();
	client._fd = -1;

	server.removeClient(client);
}

/** @todo is this necessary? */
// cleanup EVERYTHING at end of monitoring loop
// what - how - etc.. need to check this
void		Epoll::cleanUp()
{
	
}
