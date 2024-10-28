/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/28 17:08:12 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket
{
	private:
		int					_sockfd;
		int					_connection;
		//std::vector<int>	_connections;
		sockaddr_in			_sockaddr;
		unsigned long		_addrlen;
		
		
	public:
		Socket();
		Socket(const Socket &socket);
		Socket(const Webserv &servers);
		Socket	&operator=(const Socket &Socket);
		~Socket();


	/* methods */

		int		openSockets();
		void	closeSockets();
};


#endif /* SOCKET_HPP */