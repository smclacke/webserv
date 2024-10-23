/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   init.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:27:49 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/23 16:43:33 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

void initDefault(void)
{
	Webserv server;
}

void initConf(std::string conf)
{
	Webserv server(conf);
}