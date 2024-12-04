/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/04 14:52:33 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"
#include "web.hpp"

class Webserv;
class Server;
class Socket;

enum class eSocket;

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;

enum class clientState
{
	BEGIN = 0,				// ready to read/write
	READING = 1,			// currently reading
	WRITING = 2,			// currently writing
	ERROR = 3,				// something wrong
	READY = 4,				// finished reading/writing job
	CLOSED = 5,				// connection closed
	RESPONSE = 6,			// handling response
	SENDING = 7,			// sending response
	REQUEST = 8				// handling request
};

#define MAX_EVENTS 10
#define TIMEOUT 3000 // milliseconds | 3 seconds
#define READ_BUFFER_SIZE 5
#define WRITE_BUFFER_SIZE 5
#define MAX_FILE_READ 256

typedef struct s_clients
{
	int												_fd;
	struct sockaddr_in								_addr;
	socklen_t										_addLen;
	enum clientState								_clientState;
	std::unordered_map<int, timePoint>				_clientTime;
	bool											_connectionClose;
	int												_clientId;
	
	// read
	/** @todo want request to be stringstream for speed | ostringstream */
	//std::unique_ptr<std::stringstream>				_request;
	std::string										_request;
	
	// write
	std::string										_response;
	size_t											_write_offset;
	ssize_t											_bytesWritten;

}				t_clients;

typedef struct s_serverData
{
	std::shared_ptr<Server>							_server;
	std::deque<t_clients> 							_clients;

	/* methods */
	void								addClient(int sock, struct sockaddr_in &addr, int len);
	void								removeClient(t_clients &client);
}				t_serverData;

class Epoll
{
	private:
		int								_epfd;
		std::vector<t_serverData>		_serverData;
		int								_numEvents;
		struct epoll_event				_event;
		std::vector<epoll_event>		_events;
		int								_pipefd[2]; // pipe[0] read | pipe[1] write

public:
	Epoll();
	Epoll(const Epoll &copy);
	Epoll &operator=(const Epoll &epoll);
	~Epoll();

		/* methods */
		void							initEpoll();
		void							handleFile();
		void							handleRead(t_clients &client);
		void							handleWrite(t_clients &client);
		void							makeNewConnection(int fd, t_serverData &server);
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
		void							addFile();
		void							addToEpoll(int fd);
		struct epoll_event				addServerSocketEpoll(int sockfd);
		void							modifyEvent(int fd, uint32_t events);
		void							setNonBlocking(int connection);
		void							closeDelete(int fd);
		void							updateClientClock(t_clients &client);
		void							clientTimeCheck(t_clients &client);
		void							handleClientClose(t_serverData &server, t_clients &client);
		void							cleanUp();
};

#endif /* EPOLL_HPP */