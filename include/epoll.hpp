/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/15 14:50:16 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP


class Webserv;
class Socket;
enum class eSocket;

#define MAX_EVENTS 10

typedef struct s_fds
{
	int					_serverfd;
	int					_clientfd;
	socklen_t			_serveraddlen;
	struct sockaddr_in	_serveraddr;
	struct epoll_event	_event;
	struct epoll_event	_events[MAX_EVENTS];
	int					_newfd;
	socklen_t			_newaddlen;
	struct sockaddr_in	_newaddr;

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
		void					monitor(Socket &server, Socket &client, size_t i);
		void					serverSockConnect(Socket &server, t_fds fd);
		void					readClient(t_fds fd, int i);
		void					sendResponse(t_fds fd, int i);

		
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
