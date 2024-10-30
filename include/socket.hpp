/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 18:38:43 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

class Webserv;
class Server;

enum class eSocket;

class Socket
{
	private:
		int					_sockfd;
		size_t				_maxConnections;
		int 				_connection; // for grabbing one from queue
		std::vector<int>	_connections;
		sockaddr_in 		_sockaddr;
		unsigned long		_addrlen;
		

	public:
		Socket();
		Socket(const Server &servInstance, eSocket type);
		// copy constructor disappeared
		Socket &operator=(const Socket &Socket);
		~Socket();

		/* getters */

		/* setters */

		/* methods */
		int openClientSocket(const Server &serInstance);
		int openServerSocket(const Server &serInstance);
		void closeSockets(); // not sure where you will be yet, maybe in epoll
};

#endif /* SOCKET_HPP */
