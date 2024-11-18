/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/18 14:21:07 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"

class Webserv;
class Socket;
enum class eSocket;

#define MAX_EVENTS 10

typedef struct s_connections
{
	int							_conFd;
	socklen_t					_conAddLen;
	struct sockaddr_in			_conAddr;
}				t_connections;

typedef struct s_fds
{
	int							_serverfd;
	int							_clientfd;
	socklen_t					_serveraddlen;
	struct sockaddr_in			_serveraddr;
	struct epoll_event			_event;
	struct epoll_event			_events[MAX_EVENTS];
	std::vector<t_connections> 	_connection;
}				t_fds;

class Epoll
{
	private:
		int					_epfd;
		std::vector<t_fds>	_fds;
		int					_numEvents;


	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		/* methods */
		void					initEpoll();
		void					connectClient(t_fds fd);
		void					makeNewConnection(std::shared_ptr<Socket> &server, t_fds fd);
		void					readIncomingMessage(t_fds fd, int i);
		void					sendOutgoingResponse(t_fds fd, int i);

		/* getters */
		int						getEpfd() const;
		std::vector<t_fds>		getAllFds() const;
		t_fds					getFd(size_t i) const;
		int						getNumEvents() const;

		/* setters */
		void					setEpfd(int fd);
		void					setFd(t_fds fd);
		void					setNumEvents(int numEvents);

		/* utils -> epoll_utils.cpp */
		std::string				generateHttpResponse(const std::string &message);
		struct epoll_event		addSocketEpoll(int sockfd, int epfd, eSocket type);
		void					addConnectionEpoll(int connection, int epfd, struct epoll_event event);
		void					switchOUTMode(int fd, int epfd, struct epoll_event event);
		void					switchINMode(int fd, int epfd, struct epoll_event event);
		void					setNonBlocking(int connection);
		void					closeDelete(int fd, int epfd);
};

#endif /* EPOLL_HPP */