/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   run.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:25:39 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/22 17:45:50 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

void run(void)
{
	Webserv			config("input"); // will be passed to run from main

	Socket	sock(config);

	// poll();

	sock.closeSockets();
}
