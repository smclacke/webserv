/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 18:12:44 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP


class Webserv;
class Socket;
enum class eSocket;

#define MAX_EVENTS 10

class Epoll
{
	private:
		int					_epfd;
		int					_serverfd;
		int					_clientfd;
		int					_numEvents;
		socklen_t			_serveraddlen;
		struct sockaddr_in	_serveraddr;
		struct epoll_event	_event;
		struct epoll_event	_events[MAX_EVENTS];
		int					_newfd;
		socklen_t			_newaddlen;
		struct sockaddr_in	_newaddr;

	
	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		/* methods */
		void					initEpoll();
		void					connectClient();
		void					monitor(Socket &server, Socket &client);
		void					serverSockConnect(Socket &client);
		void					readClient(int i);
		void					sendResponse(int i);

		
		/* getters */
		int						getEpfd() const;
		int						getServerfd() const;
		int						getClientfd() const;
		int						getNumEvents() const;
		socklen_t				getServeraddlen() const;
		struct sockaddr_in		getServeraddr() const;

		/* setters */
		void					setEpfd(int fd);
		void					setServerfd(int fd);
		void					setClientfd(int fd);
		void					setNumEvents(int numEvents);
		void					setServeraddlen(socklen_t addrlen);
		void					setServeraddr(struct sockaddr_in addr);

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
