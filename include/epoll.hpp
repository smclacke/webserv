/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/04 15:45:04 by eugene        ########   odam.nl         */
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
		//class Socket|int	_sockfd; // need both server and client sockets
		int					_epfd; // need you and _fd?
		int					_op;
		int					_fd; // need you and _epfd?
		int					_numEvents;
		int					_timeout;
		struct epoll_event	*_event;
		struct epoll_event	*_events;
		struct epoll_event	_clientEvent;
		

	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		///* getters */

		///* setters */

		///* methods */
		void	initEpoll();
		void	monitor();

};

#endif /* EPOLL_HPP */
