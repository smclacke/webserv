/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 18:38:34 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP


class Webserv;
enum class eSocket;

class Epoll
{
	private:
		//int					_epfd;
		//int					_op;
		//int					_fd;
		//int					_nb; // for create but maybe not necessary
		//int					_maxevents;
		//int					_timeout;
		//struct epoll_event	*_event;
		//struct epoll_event	*_events;

	public:
		Epoll();
		// copy constructor
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		///* getters */

		///* setters */

		///* methods */
		void	initEpoll();

};

#endif /* EPOLL_HPP */
