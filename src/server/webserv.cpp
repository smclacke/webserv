/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 15:59:25 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

/**
 * @brief default constructor in case no config file was provided.
 */
Webserv::Webserv(void)
{
	Server default_server;
	_servers.push_back(default_server);
}

Webserv::Webserv(std::string config)
{
	if (config.empty())
	{
		Server default_server;
		_servers.push_back(default_server);
		return;
	}
	// parse config file
	// if _server.size == 0 --> config file was empty --> add defualt server
	(void)config;
}

Webserv::~Webserv(void)
{
}

/* member functions */

void Webserv::start(void)
{
}

/* setters */

void Webserv::addServer(Server &server)
{
	_servers.push_back(server);
}

/* getters */

std::vector<Server> &Webserv::getallServer()
{
	return (_servers);
}

size_t Webserv::getServerCount(void) const
{
	return (_servers.size());
}

Server &Webserv::getServer(size_t index)
{
	return (_servers.at(index));
}

Server &Webserv::getServer(std::string name)
{
	auto it = std::find_if(_servers.begin(), _servers.end(), [&name](const Server &server)
						   {
							   return server.getServerName() == name; // Assuming you have a method to get the server name
						   });
	if (it != _servers.end())
	{
		return *it; // Return the found server
	}
	throw std::runtime_error("Server not found"); // Handle the case where the server is not found
}