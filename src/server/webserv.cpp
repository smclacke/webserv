/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/05 14:51:55 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include "../../include/error.hpp"

/* constructors */
/**
 * @brief default constructor in case no config file was provided.
 */
Webserv::Webserv(std::atomic<bool>  &keepRunning) : _keepRunning(keepRunning)
{
	std::cout << "Webserv booting up" << std::endl;
	auto default_server = std::make_shared<Server>();
	_servers.push_back(default_server);
}

Webserv::Webserv(std::string config, std::atomic<bool> &keepRunning) : _keepRunning(keepRunning)
{
	std::cout << "Webserv booting up" << std::endl;
	_epoll.initEpoll();
	if (config.empty())
	{
		std::cout << "Default configuration\n\n";
		auto default_server = std::make_shared<Server>();
		_servers.push_back(default_server);
		auto default_server2 = std::make_shared<Server>(9999);
		_servers.push_back(default_server2);
		return;
	}
	std::cout << "config: " << config << std::endl << "\n";
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
void Webserv::addServersToEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::shared_ptr<Server>		currentServer = getServer(i);
		//t_serverData				thisServer;
		//thisServer._server = currentServer; // was this ever doing anything?
		int					serverSockfd = currentServer->getServerSocket()->getSockfd();
		struct epoll_event 	event;

		event.data.fd = serverSockfd;
		_epoll.addServerSocketEpoll(serverSockfd);
		_epoll.setEvent(event);
		_epoll.setServer(currentServer);
	}
	std::cout << "--------------------------\n";
}

void		Webserv::monitorServers()
{
	std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~\n";
	std::cout << "Entering monitoring loop\n";
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~\n";

	while (_keepRunning)
	{
		for (auto &servers : _epoll.getAllServers())
		{
			for (auto &client : servers._clients)
				_epoll.clientTimeCheck(client);
		}
		int numEvents = epoll_wait(_epoll.getEpfd(), _epoll.getAllEvents().data(), _epoll.getAllEvents().size(), TIMEOUT);
		if (numEvents == -1)
		{
			if (_keepRunning == false)
				return ;
			throw std::runtime_error("epoll_wait() failed\n");
		}
		else if (numEvents == 0)
			continue ;
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = _epoll.getAllEvents()[i].data.fd;
			_epoll.processEvent(fd, _epoll.getAllEvents()[i]);
		}
	}
	_epoll.cleanUp();
}

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

std::vector<std::shared_ptr<Server>> &Webserv::getAllServers()
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
							   return server->getServerName() == name;
						   });
	if (it != _servers.end())
	{
		return *it;
	}
	throw std::runtime_error("Server not found");
}

Epoll &Webserv::getEpoll()
{
	return this->_epoll;
}
