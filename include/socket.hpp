/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 13:52:13 by smclacke      ########   odam.nl         */
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
		std::vector<int>		_connections; // need vector of connections? [LIST]
		struct sockaddr_in		_sockaddr; // socket address
		socklen_t				_addrlen; // socket address length
		int						_reuseaddr; // for re-binding to port while preivous
											// connection is still in TIME_WAIT state
		int						_flags; // setting nonblocking on the socket
		std::string				_host;

	public:
		Socket();
		Socket(const Server &servInstance, eSocket type);
		Socket(const Socket &copy);
		Socket &operator=(const Socket &Socket);
		~Socket();

		/* methods */
		void					openClientSocket(const Server &serInstance);
		void					openServerSocket(const Server &serInstance);
		void					closeSocket();

		/* getters */
		int						getSockfd() const;
		int						getConnection() const;
		struct sockaddr_in		getSockaddr() const;
		socklen_t				getAddrlen() const;
		std::string				getHost() const;

		/* setters */
		void					setSockfd(int fd);
		void					setNewConnection(int &connection);
		void					setSockaddr(struct sockaddr_in &sockaddr);
		void					setAddrlen(socklen_t &addrlen);
		void					setHost(std::string host);

};

#endif /* SOCKET_HPP */
