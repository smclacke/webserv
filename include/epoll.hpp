/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 19:47:44 by smclacke      ########   odam.nl         */
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
		int					_op;
		int					_numEvents;
		int					_timeout;
		//struct epoll_event	_event;
		

	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();


		///* getters */

		///* setters */

		///* methods */
		int		initEpoll();
		int		monitor(Socket &client, Socket &server);

};

#endif /* EPOLL_HPP */
