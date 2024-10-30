/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:40:39 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 17:53:55 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "webserv.hpp"
#include "server.hpp"

class Webserv;
enum class eSocket;

class Epoll
{
	private:
		//event
		// op
		// fd
		
		

	public:
		Epoll();
		// copy constructor
		Epoll &operator=(const Epoll &epoll);
		~Epoll();

		///* getters */

		///* setters */

		///* methods */
		void	initEpoll();
		// event

};

#endif /* EPOLL_HPP */
