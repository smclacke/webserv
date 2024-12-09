/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/09 15:19:00 by smclacke      ########   odam.nl         */
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
	if (config.empty())
	{
		std::cout << "Default configuration\n\n";
		config = "./config_files/test.conf";
	}
	std::cout << "Config: " << config << std::endl << "\n";
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
				//checkDoublePorts();
				return;
			}
			continue;
		}
		else
			throw eConf("line : \"" + line + "\": not recognized", line_n);
	}
	//checkDoublePorts();
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
		int							serverSockfd = currentServer->getServerSocket()->getSockfd();
		struct epoll_event 			event;

		event.data.fd = serverSockfd;
		_epoll.addToEpoll(serverSockfd);
		_epoll.setEvent(event);
		_epoll.setServer(currentServer);
	}
}

void Webserv::removeServersFromEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::shared_ptr<Server>		currentServer = getServer(i);
		int							serverSockfd = currentServer->getServerSocket()->getSockfd();
		
		if (epoll_ctl(_epoll.getEpfd(), EPOLL_CTL_DEL, serverSockfd, nullptr) == -1)
			std::cerr << "Failed to remove fd from epoll\n";
	}
}

void		Webserv::monitorServers()
{
	_epoll.initEpoll();
	addServersToEpoll();

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
			removeServersFromEpoll();
			throw std::runtime_error("epoll_wait()\n");
		}
		else if (numEvents == 0)
			continue ;
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = _epoll.getAllEvents()[i].data.fd;
			_epoll.processEvent(fd, _epoll.getAllEvents()[i]);
		}
	}
	removeServersFromEpoll();
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

/**
 * @brief verifies that there are no two ports the same in Webserv
 * @note this is checked already in bind() since function will fail if trying to bind to
 * 	an already in use address
 */
//void Webserv::checkDoublePorts()
//{
//	std::unordered_set<int> portSet;
//	for (const auto &serv : _servers)
//	{
//		int Port = serv->getPort();
//		if (portSet.find(Port) != portSet.end())
//		{
//			throw eConf("Duplicate server port found: " + std::to_string(Port), 0);
//		}
//		portSet.insert(Port);
//	}
//}
