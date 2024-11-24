/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/24 16:32:39 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

/**
 * @todo removed http response function and use Julius' stuff
 * @todo check closeDelete order
 */

/* Epoll utils */
std::string Epoll::generateHttpResponse(const std::string &message)
{
	size_t	contentLength = message.size();
	std::ostringstream	response;
	response	<< "HTPP/1.1 200 OK\r\n"
				<< "Content-Type: text/plain\r\n"
				<< "Content-Length: " << contentLength << "\r\n"
				<< "Connection : close\r\n"
				<< "\r\n"
				<< message;
	
	return response.str();
}

struct epoll_event Epoll::addSocketEpoll(int sockfd, int epfd, eSocket type)
{
	struct epoll_event	event;
	event.data.fd = sockfd;
	if (type == eSocket::Server)
		event.events = EPOLLIN;
	else if (type == eSocket::Client)
		event.events = EPOLLIN;
		//event.events = EPOLLIN | EPOLLOUT;
	else
		throw std::runtime_error("invalid socket type");

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		protectedClose(sockfd);
		throw std::runtime_error("Error adding socket to epoll\n");
	}
	return event;
}

void		Epoll::addToEpoll(int fd, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		protectedClose(fd);
		throw std::runtime_error("Error adding fd to epoll\n");
	}
	std::cout << "New fd added to epoll\n";
}

void		Epoll::modifyEvent(int fd, int epfd, uint32_t events)
{
	struct epoll_event event;

	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		protectedClose(fd);
		throw std::runtime_error("Failed to modify socket event type\n");
	}
	std::cout << "Modified socket event type\n";
	
}

void		Epoll::setNonBlocking(int connection)
{
	int	flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void		Epoll::closeDelete(int fd, int epfd)
{
	protectedClose(fd);
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}
