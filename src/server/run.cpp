/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   run.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:25:39 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 15:37:42 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

void run(Webserv wserv)
{
	// epoll first then assign servers and socket to the instance of epoll()
	// initEpoll()


	Socket	sock(wserv);

	// assign sockets/servers to Epoll monitoring

	sock.closeSockets();
}
