/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/11 15:32:40 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"
#include "web.hpp"
#include "httpHandler.hpp"

class Webserv;
class Server;
class Socket;
class httpHandler;
class Epoll;

enum class eSocket;

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;

enum class clientState
{
	BEGIN = 0,	 // ready to read/write
	READING = 1, // currently reading
	WRITING = 2, // currently writing
	ERROR = 3,	 // something wrong - dont actually use since client gets closed
	READY = 4,	 // finished reading/writing job
	CLOSE = 5,	 // connection can be closed
	CLOSED = 6	 // has been closed, should also be removed, if not, removal failed
};

#define MAX_EVENTS 10
#define TIMEOUT 3000 // milliseconds | 3 seconds
#define READ_BUFFER_SIZE 100
#define WRITE_BUFFER_SIZE 100
#define MAX_FILE_READ 256

typedef struct s_clients
{
	int										_fd;
	struct sockaddr_in 						_addr;
	socklen_t 								_addLen;
	enum clientState						_clientState;
	std::unordered_map<int, timePoint>		_clientTime;
	bool									_connectionClose;

	std::shared_ptr<httpHandler>			http;
	//std::string								_requestClient; // what has julius done

	/** Write */
	s_httpSend								_responseClient;
	size_t									_write_offset;
	ssize_t									_bytesWritten;
	bool									_readingFile;
	s_clients(Epoll &epoll, Server &server);
	~s_clients() {};

} t_clients;

typedef struct s_serverData
{
	std::shared_ptr<Server>					_server;
	std::deque<t_clients>					_clients;

	/* methods */
	void								addClient(int sock, struct sockaddr_in &addr, int len, Epoll &epoll);
	void								removeClient(t_clients &client);
	
} 				t_serverData;
class Epoll
{
	private:
		int										_epfd;
		std::vector<t_serverData>				_serverData;
		int										_numEvents;
		struct epoll_event						_event;
		std::vector<epoll_event>				_events;
		struct s_cgi							&cgi;
		httpHandler								&cgi_http;

	public:
		Epoll();
		~Epoll();

		/* methods */
		void							initEpoll();
		//s_httpSend						handleRequest(std::string &request, Server &server);
		void							closeAllPipes(int cgiIN[2], int cgiOUT[2]);
		void							handleCgiRead(httpHandler &cgi_http, int cgiOUT);
		void							handleCgiWrite(httpHandler &cgi_http, int cgiIN);
		void							handleRead(t_clients &client);
		void							handleWrite(t_clients &client);
		void							handleFile(t_clients &client);
		void							makeNewConnection(int fd, t_serverData &server);
		void							processEvent(int fd, epoll_event &event);

		/* getters */
		int								getEpfd() const;
		int								getNumEvents() const;
		std::vector<t_serverData>		&getAllServers();
		std::shared_ptr<Server>			getServer(size_t i);
		std::vector<epoll_event>		&getAllEvents();
		struct epoll_event				&getEvent();

		/* setters */
		void							setEpfd(int fd);
		void							setNumEvents(int numEvents);
		void							setEventMax();
		void							setEvent(struct epoll_event &event);
		void							setServer(std::shared_ptr<Server>);

		
		/* utils -> epoll_utils.cpp */
		void							addToEpoll(int fd);
		void							addOUTEpoll(int fd);
		void							modifyEvent(int fd, uint32_t events);
		void							setNonBlocking(int connection);
		void							updateClientClock(t_clients &client);
		void							clientTimeCheck(t_clients &client);
		void							closeDelete(int fd);
		void							handleClientClose(t_serverData &server, t_clients &client);
		void							operationFailed(t_clients &client);
};

#endif /* EPOLL_HPP */
