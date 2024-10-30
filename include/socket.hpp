/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 16:34:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 16:32:31 by jde-baai      ########   odam.nl         */
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
	int _sockfd;
	int _connection;
	// std::vector<int>	_connections;
	sockaddr_in _sockaddr;
	unsigned long _addrlen;

public:
	Socket();
	Socket(eSocket type);
	Socket(const Webserv &servers);
	Socket &operator=(const Socket &Socket);
	~Socket();

	/* methods */

	int openSockets();
	void closeSockets();
	void initEpoll(Socket &sock);
};

#endif /* SOCKET_HPP */
