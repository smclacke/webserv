/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 14:07:39 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include "../../include/error.hpp"

/* constructors */

/**
 * @brief default constructor in case no config file was provided.
 */
Webserv::Webserv(void)
{
	std::cout << "Webserv booting up" << std::endl;
	auto default_server = std::make_shared<Server>();
	_servers.push_back(default_server);
}

Webserv::Webserv(std::string config)
{
	std::cout << "Webserv booting up" << std::endl;
	std::cout << "config: " << config << std::endl;
	if (config.empty())
	{
		auto default_server = std::make_shared<Server>();
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
			auto nServer = std::make_shared<Server>(file, line_n);
			_servers.push_back(nServer);
			if (_servers.size() == 10)
			{
				std::cerr << "\033[1;31mwarning: max number of servers(10) added, stopped reading conf\033[0m" << std::endl;
				return;
			}
			continue;
		}
		else
			throw eConf("line : \"" + line + "\": not recognized", line_n);
	}
}

Webserv::~Webserv(void)
{
	std::cout << "Webserv shutting down" << std::endl;
}

/* member functions */


/* setters */

void Webserv::addServer(std::shared_ptr<Server> server)
{
	_servers.push_back(server);
}

/* getters */

std::shared_ptr<Server> Webserv::getServer(size_t index)
{
	return _servers[index];
}

std::vector<std::shared_ptr<Server>> &Webserv::getallServer()
{
	return _servers;
}

size_t Webserv::getServerCount(void) const
{
	return _servers.size();
}

std::shared_ptr<Server> Webserv::getServer(std::string name)
{
	auto it = std::find_if(_servers.begin(), _servers.end(), [&name](const std::shared_ptr<Server> &server)
						   {
							   return server->getServerName() == name; // Assuming you have a method to get the server name
						   });
	if (it != _servers.end())
	{
		return *it; // Return the found server
	}
	throw std::runtime_error("Server not found"); // Handle the case where the server is not found
}