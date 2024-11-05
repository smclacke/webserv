/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 16:57:57 by smclacke      ########   odam.nl         */
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
		size_t					_maxConnections; // set to 10
		int 					_connection; // for grabbing one from queue
		std::vector<int>		_connections; // need vector of connections?
		struct sockaddr_in		_sockaddr; // socket address
		socklen_t				_addrlen; // socket address length
		int						_reuseaddr; // for re-binding to port while preivous
											// connection is still in TIME_WAIT state
		int						_flags; // setting nonblocking on the socket

	public:
		Socket();
		Socket(const Server &servInstance, eSocket type);
		Socket(const Socket &copy);
		Socket &operator=(const Socket &Socket);
		~Socket();


		/* getters */
		int						getSockFd() const;
		int						getConnection() const;
		struct sockaddr_in		getSockaddr() const;
		socklen_t				getAddrlen() const;

		/* setters */
		void			setSockFd(int fd);
		void			setNewConnection(int &connection);
		void			setSockaddr(struct sockaddr_in &sockaddr);
		void			setAddrlen(socklen_t &addrlen);

		/* methods */
		int				openClientSocket(const Server &serInstance);
		int				openServerSocket(const Server &serInstance);
		void			closeSocket();
};

#endif /* SOCKET_HPP */
