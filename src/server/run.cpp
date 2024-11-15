/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   run.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:25:39 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/15 18:14:54 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/socket.hpp"
#include "../../include/webserv.hpp"

void run(Webserv wserv)
{
	// epoll first then assign servers and socket to the instance of epoll()
	// initEpoll()

	Socket sock(wserv);

	// assign sockets/servers to Epoll monitoring

	sock.closeSockets();
}
