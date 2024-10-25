/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   init.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:27:49 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/25 15:28:40 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

void initDefault(void)
{
	Webserv server;

	Server test = server.getServer((std::string("default_server")));
	test.printServer();
}

void initConf(std::string conf)
{
	Webserv server(conf);
}