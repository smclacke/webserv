/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/06 18:27:56 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "web.hpp"
#include "server.hpp"

class Socket
{
	private:
		int						_sockfd;
		size_t					_maxConnections;
		struct sockaddr_in		_sockaddr;
		socklen_t				_addrlen;
		int						_reuseaddr;
		int						_flags;

	public:
		Socket();
		Socket(const Server &servInstance);
		~Socket();

		/* methods */
		void					openServerSocket(const Server &serInstance);

		/* getters */
		int						getSockfd() const;
		struct sockaddr_in		getSockaddr() const;
		socklen_t				getAddrlen() const;

		/* setters */
		void					setSockfd(int fd);
		void					setSockaddr(struct sockaddr_in &sockaddr);
		void					setAddrlen(socklen_t &addrlen);

};

#endif /* SOCKET_HPP */