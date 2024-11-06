/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 17:44:53 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

class Webserv;
class Server;

enum class eSocket;

#define BUFFER_SIZE 1000

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
		Socket(const Server &servInstance, eSocket type);
		Socket(const Socket &copy);
		Socket &operator=(const Socket &Socket);
		~Socket();

		/* methods */
		void					openServerSocket(const Server &serInstance);
		void					openClientSocket();

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
