/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/04 16:12:55 by eugene        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors an' that */

Epoll::Epoll() {}


Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll &Epoll::operator=(const Epoll &epoll)
{
	if (this != &epoll)
	{
		this->_epfd = epoll._epfd;
		//this->_sockfd = epoll._sockfd;
	}
	return *this;
}

Epoll::~Epoll() {}


/* methods */

void	Epoll::initEpoll()
{
	//_epfd = epoll_create(0);
	//_event->events = EPOLLIN;
	//_event->data.fd = _sockfd;
	//epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &_event);
}
// Server class has client and server sockets, use those?
void	Epoll::monitor()
{
	//while (true)
	//{
	//	//_events[MAX_EVENTS];
	//	_numEvents = epoll_wait(_epfd, events, MAX_EVENTS, -1);
		
	//	for (int i = 0; i < _numEvents; ++i)
	//	{
	//		if (_events[i].data.fd == _sockfd)
	//		{	
	//			// accept connection
	//			_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
	//			if (_connection < 0)
	//			{
	//				if (errno == EWOULDBLOCK || errno == EAGAIN)
	//					continue ;
	//				std::cerr << "error accepting connection\n";
	//				continue ;
	//			}
	//			// TODO: set connection to non-blocking

	//			// add client to epoll (need to check this in connection with opening the client socket)
	//			_clientEvent.events = EPOLLIN;
	//			_clientEvent.data.fd = _sockfd; // client fd - how to separate ...
	//			epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &_clientEvent);
				
	//		}
	//		else	
	//			// handle data from client socket
	//			// client_fd = events[i].data.fd;
	//			// read and process data
	//	}
	//}
}


// CHECK NOTES THEN MOVE

/**
 * monitoring serveral sockets
 * waits for changing state or changing level for each socket monitored
 * handles lots of socket descriptors
 * 
 * contains internal structure: interest list (corresponding to all FDs monitors), a ready list (corresponding to FDs ready for I/O)
 * 
 * 
 * ---- functions ----
 * 
 * creates new epoll instance and reutrns a descriptor
 * < int		epoll_create(int nb); >
 * 
 * 
 * changes the behaviour of epoll instance
 * epfd: descriptor of epoll instance created
 * op: operation wanted on epoll structure (e.g. add new fd in interest list, modify/delete)
 * fd: concerned descriptor
 * event: filled with concerned fd and flags to apply to fd
 * < int		epoll_ctl(int epfd, int op, int fd, struct epoll_event *event); >
 * 
 * 
 * 
 * waits for event on any descriptor in interest list
 * funtion will block until - fd delivers event | call is interrupted by signal handler | timeout expires
 * epfd: descriptor of epoll instance created
 * maxevents: max of events returned
 * events: return info from ready list
 * < int		epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout); >
 * 
 * 
 * 
 */

// change close sockets, think it happens all here

//void	Socket::initEpoll(Socket &sock)
//{
//	(void) sock;
//	// create socketEpoll(sock)

//	// while waitEpoll != err
	
//	// loop getsocketnumber
//		// close getsocket
//		// epollctl(getepollfd, get socket etc...)
//		// close sock getepollfd

//}

