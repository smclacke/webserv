/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 14:34:27 by smclacke      ########   odam.nl         */
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
		struct epoll_event	_event;
		
		// instead of being in the socket class - not sure about this yet but
		// connections are made in the main epoll loop sooo...
		int 					_connection; // for grabbing one from queue
		std::vector<int>		_connections; // need vector of connections?

		struct epoll_event	*_events; // ?
		struct epoll_event	_clientEvent; // ?
		

	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();


		///* getters */

		///* setters */

		///* methods */
		int		initEpoll();
		int		monitor(const Socket &client, const Socket &server);

};

#endif /* EPOLL_HPP */
