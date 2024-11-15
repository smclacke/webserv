/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/15 15:56:58 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

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
	std::string		sortSocket;

	if (type == eSocket::Server)
	{
		std::cout << "server\n";
		event.events = EPOLLIN;
		sortSocket = "Server";
	}
	else if (type == eSocket::Client)
	{
		event.events = EPOLLIN | EPOLLOUT;
		sortSocket = "Client";
	}
	else
		throw std::runtime_error("invalid socket type passed as argument\n");
	
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		protectedClose(sockfd);
		throw std::runtime_error("Error adding socket to epoll\n");
	}
	std::cout << "Successfully added " << sortSocket << " socket to epoll\n";
	return event;
}

 void		Epoll::addConnectionEpoll(int connection, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = connection;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, connection, &event) < 0)
	{
		protectedClose(connection);
		throw std::runtime_error("Error adding connection to epoll\n");
	}
	std::cout << "Successfully added connection to epoll\n";
}


void		Epoll::switchOUTMode(int fd, int epfd, struct epoll_event event)
{
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		protectedClose(fd);
		throw std::runtime_error("Failed to modify client socket for writing\n");
	}
	std::cout << "Successfully modified client socket for writing\n";
}

void		Epoll::switchINMode(int fd, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		protectedClose(fd);
		throw std::runtime_error("Failed to modify client socket for reading\n");
	}
	std::cout << "Successfully modified client socket for reading\n";
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

