/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   run.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:25:39 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/22 17:20:40 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

// config struct passed when constructing socket class
// pass config instance to run()

void run(void)
{
	std::string		input = "nothing input"; // will remove
	Webserv			config(input); // will be passed to run from main

	Socket	sock = Socket(config);

	// initpoll();
	// pollLoop();

	sock.closeSockStuff();
}
