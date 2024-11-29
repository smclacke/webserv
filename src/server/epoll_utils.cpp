/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll_utils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 16:43:57 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/29 17:57:51 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

void		Epoll::printAllEpoll()
{
	
}

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

struct epoll_event Epoll::addServerSocketEpoll(int sockfd)
{
	struct epoll_event	event;
	event.data.fd = sockfd;
	event.events = EPOLLIN;

	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		protectedClose(sockfd);
		throw std::runtime_error("Error adding socket to epoll\n");
	}
	return event;
}

/** @todo make this function
 * 	
 * 	pipefd[0] - read
 * 	pipefd[1] - write
 */
void	Epoll::addFile()
{
	if (pipe(_pipefd) == -1)
	{
		std::cerr << "pipe for file failed\n";
		return ;
	}

	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = _pipefd[0];

	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _pipefd[0], &event) == -1)
	{
		std::cerr << "epoll_ctl file failed\n";
		protectedClose(_pipefd[0]);
		protectedClose(_pipefd[1]);
		return ;
	}
}

void		Epoll::addToEpoll(int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		protectedClose(fd);
		throw std::runtime_error("Error adding fd to epoll\n");
	}
	std::cout << "New fd added to epoll: " << event.data.fd << "\n";
}

void		Epoll::modifyEvent(int fd, uint32_t events)
{
	struct epoll_event event;

	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		protectedClose(fd);
		throw std::runtime_error("Failed to modify socket event type\n");
	}
}

void		Epoll::setNonBlocking(int connection)
{
	int	flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

void		Epoll::closeDelete(int fd)
{
	protectedClose(fd);
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void		Epoll::updateClientClock(t_clients &client)
{
	client._clientTime[client._fd] = std::chrono::steady_clock::now();
}

/** @todo test this */
void		Epoll::clientTimeCheck(t_clients &client)
{
	auto	now = std::chrono::steady_clock::now();

	for (auto it = client._clientTime.begin(); it != client._clientTime.end();)
	{
		int client_fd = it->first;
		auto lastActivity = it->second;

		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastActivity).count() > TIMEOUT)
		{
			std::cout << "Client timed out\n";
			closeDelete(client_fd);
			it = client._clientTime.erase(it);	
		}
		else
			it++;
	}
}

/** @todo work in progress */
void		Epoll::handleClose(t_clients &client)
{
    bool closeSuccess = true;

    try
	{
		if (client._fd != -1)
		{
            if (!protectedClose(client._fd))
			{
                closeSuccess = false;
                std::cerr << "Failed to close client socket " << client._fd << ": " << strerror(errno) << std::endl;
            }
			else
                std::cout << "Closed client socket " << client._fd << std::endl;
            client._fd = -1;
        }
    }
	catch (const std::exception &e)
	{
        std::cerr << "Exception during close: " << e.what() << std::endl;
        closeSuccess = false;
    }

    if (!closeSuccess)
		closeDelete(client._fd);

    client._clientState = clientState::CLOSED;
    client._request.clear();
    client._response.clear();
}
