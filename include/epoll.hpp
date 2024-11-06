/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 14:54:21 by smclacke      ########   odam.nl         */
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
		//int					_timeout;
		
	public:
		Epoll();
		Epoll(const Epoll &copy);
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		/* methods */
		void					initEpoll();
		std::string				generateHttpResponse(const std::string &message);
		struct epoll_event		addSocketEpoll(int sockfd, int epfd, eSocket type);
		void					addConnectionEpoll(int connection, int epfd, struct epoll_event event);
		void					setNonBlocking(int connection);
		void					closeDelete(int fd, int epfd);
		void					switchReadMode(int fd, int epfd, struct epoll_event event);
		void					monitor(Socket &client, Socket &server);
		
		/* getters */

		/* setters */


};

#endif /* EPOLL_HPP */
