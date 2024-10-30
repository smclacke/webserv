/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 17:39:45 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "webserv.hpp"
#include "server.hpp"

class Webserv;
enum class eSocket;

class Socket
{
	private:
		int					_sockfd;
		int					_maxConnections;
		int 				_connection; // for grabbing one from queue
		std::vector<int>	_connections;
		sockaddr_in 		_sockaddr;
		unsigned long		_addrlen;
		

	public:
		Socket();
		Socket(const Server &serv_instance, eSocket type);
		// copy constructor disappeared
		Socket &operator=(const Socket &Socket);
		~Socket();

		/* getters */

		/* setters */

		/* methods */
		int openClientSocket(const Server &sev_instance);
		int openServerSocket(const Server &sev_instance);
		void closeSockets(); // not sure where you will be yet
};

#endif /* SOCKET_HPP */
