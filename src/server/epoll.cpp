/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 15:43:16 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

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

void	Socket::initEpoll(Socket &sock)
{
	(void) sock;
	// create socketEpoll(sock)

	// while waitEpoll != err
	
	// loop getsocketnumber
		// close getsocket
		// epollctl(getepollfd, get socket etc...)
		// close sock getepollfd

}

