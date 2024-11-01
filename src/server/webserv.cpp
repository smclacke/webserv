/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/01 15:44:03 by jde-baai      ########   odam.nl         */
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
	std::cout << "config: " << config << std::endl;
	if (config.empty())
	{
		Server default_server;
		_servers.push_back(default_server);
		return;
	}
	std::ifstream file(config);
	if (!file.is_open())
		throw std::runtime_error("unable to open file: \"" + config + "\"");
	std::string line;
	int line_n = 0; // keeps track of the line_number for accurate error outputs
	while (std::getline(file, line))
	{
		++line_n;
		lineStrip(line);
		if (line.empty()) // skip empty lines
			continue;
		if (line.find("server") != std::string::npos || line.find("Server") != std::string::npos) // check both
		{
			Server nServer = Server(file, line_n);
			_servers.push_back(nServer);
			continue;
		}
		else
			throw eConf("line : \"" + line + "\": not recognized", line_n);
	}
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