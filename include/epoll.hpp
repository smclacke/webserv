/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/25 12:18:55 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"

class Webserv;
class Socket;

enum class eSocket;

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;

enum class clientState
{
	PARSING = 0,
	BEGIN = 1,
	READING = 2,
	WRITING = 3,
	ERROR = 4,
	READY = 5,
	RESPONSE = 6,
	SENDING = 7
};

#define MAX_EVENTS 10
#define TIMEOUT 3000 // milliseconds | 3 seconds
#define READ_BUFFER_SIZE 4096
#define WRITE_BUFFER_SIZE 4096

typedef struct s_clients
{
	int							_fd;
	struct sockaddr_in			_addr;
	socklen_t					_addLen;
}				t_clients;

typedef struct s_serverData
{
	std::shared_ptr<Server>							_server;
	std::unordered_map<int, timePoint>				_clientTime;
	enum clientState								_clientState;
	std::vector<t_clients> 							_clients;		// connections for server socket to accept

	/* methods */
	void								addClient(int sock, struct sockaddr_in addr, int len);
	void								setClientState(enum clientState state);
	enum clientState					getClientState();
}				t_serverData;

class Epoll
{
	private:
		int								_epfd;
		std::vector<t_serverData>		_serverData;
		int								_numEvents;
		struct epoll_event				_event;
		std::vector<epoll_event>		_events;

	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		/* methods */
		void							initEpoll();
		void							resizeEventBuffer(int size);
		void							clientTime(t_serverData server);
		void							connectClient(t_serverData server);
		void							handleClose(t_serverData &server, t_clients &client);
		void							handleFile();
		void							handleRead(t_serverData &server, t_clients &client);
		void							handleWrite(t_serverData &server, t_clients &client);
		void							makeNewConnection(int fd, t_serverData server);
		void							processEvent(int fd, epoll_event &event);
		
		/* getters */
		int								getEpfd() const;
		std::vector<t_serverData>		&getAllServers();
		std::shared_ptr<Server>			getServer(size_t i);
		int								getNumEvents() const;
		std::vector<epoll_event>&		getAllEvents();
		struct epoll_event				&getEvent();

		/* setters */
		void							setEpfd(int fd);
		void							setServer(std::shared_ptr<Server>);
		void							setNumEvents(int numEvents);
		void							setEventMax();
		void							setEvent(struct epoll_event &event);

		/* utils -> epoll_utils.cpp */
		std::string						generateHttpResponse(const std::string &message);
		struct epoll_event				addSocketEpoll(int sockfd, int epfd, eSocket type);
		void							addToEpoll(int fd, int epfd, struct epoll_event event);
		void							modifyEvent(int fd, int epfd, uint32_t events);
		void							setNonBlocking(int connection);
		void							closeDelete(int fd, int epfd);
};

#endif /* EPOLL_HPP */