/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/18 17:09:43 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"

class Webserv;
class Socket;

enum class eSocket;

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;

enum class clientStatus
{
	PARSING = 0,
	BEGIN = 1,
	READING = 2,
	WRITING = 3,
	ERROR = 4,
	READY = 5,
	RESONSE = 6,
	SENDING = 7
};

#define MAX_EVENTS 10
#define TIMEOUT 30000 // milliseconds
#define BUFFER_SIZE 1024

typedef struct s_clients
{
	int							_conFd;
	socklen_t					_conAddLen;
	struct sockaddr_in			_conAddr;
}				t_clients;

typedef struct s_fds
{
	int											_serverfd;
	int											_clientfd; //socket
	std::unordered_map<int, timePoint>			_clientStatus;
	std::vector<t_clients> 						_clients;
	socklen_t									_serveraddlen;
	struct sockaddr_in							_serveraddr;
	struct epoll_event							_event;
	struct epoll_event							_events[MAX_EVENTS];
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
		void					clientStatus(t_fds fd);
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