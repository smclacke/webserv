/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 16:40:56 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void httpHandler::cgiResponse(void)
{
	std::cout << "It is a CGI request" << std::endl;
	return;
}

/**
 * check if file is executable
 * pipe
 * fork
 * in child:
 * 	-close read end
 * 	-Set paramaters
 * 	-execve
 * else
 * 	-close write end
 * 	- set CGI as true
 * 	- set ReadFD
 * 	- set pid
 */