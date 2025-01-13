/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/13 19:33:53 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include "../../include/error.hpp"

/* constructors */
/**
 * @brief default constructor in case no config file was provided.
 */
Webserv::Webserv(std::atomic<bool> &keepRunning) : _keepRunning(keepRunning)
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
	std::cout << "Config: " << config << std::endl
			  << "\n";
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
	for (auto server : _servers)
	{
		int serverSockfd = server->getServerSocket()->getSockfd();
		struct epoll_event event;
		event.data.fd = serverSockfd;
		_epoll.addINEpoll(serverSockfd);
		_epoll.setEvent(event);
		_epoll.setServer(server);
		server->logMessage("addServersToEpoll(): added: " + server->getServerName() + " to epoll");
	}
}

void Webserv::removeServersFromEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::shared_ptr<Server> currentServer = getServer(i);
		int serverSockfd = currentServer->getServerSocket()->getSockfd();

		if (epoll_ctl(_epoll.getEpfd(), EPOLL_CTL_DEL, serverSockfd, nullptr) == -1)
			std::cerr << "Failed to remove fd from epoll\n";
	}
}

void Webserv::monitorServers()
{
	_epoll.initEpoll();
	addServersToEpoll();

	/** @note kill() causes logout :'(.. 
	*/
	
	time_t start_time, current_time;
	int timeout = 3;
	time(&start_time);

	while (_keepRunning)
	{
		for (auto &servers : _epoll.getAllServers())
		{
			for (auto &client : servers._clients)
			{
				_epoll.clientTimeCheck(client);
				if (client._clientHasCgi == true)
				{
					std::cout << "client with cgi detected correctly\n";
					time(&current_time);
					if ((current_time - start_time) >= timeout)
					{
					/** @todo  http response timeout */
						std::cout << "timeout occurred\n";

						//kill(client.cgi.pid, SIGINT);
						waitpid(client.cgi.pid, NULL, WNOHANG);
					}
				}
			}
		}
		int numEvents = epoll_wait(_epoll.getEpfd(), _epoll.getAllEvents().data(), _epoll.getAllEvents().size(), TIMEOUT);
		if (numEvents == -1 || numEvents > MAX_EVENTS)
		{
			if (_keepRunning == false)
				return;
			removeServersFromEpoll();
			throw std::runtime_error("epoll_wait()\n");
		}
		else if (numEvents == 0)
			continue;
		for (int i = 0; i < numEvents; ++i)
			_epoll.processEvent(_epoll.getAllEvents()[i].data.fd, _epoll.getAllEvents()[i]);
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
						   { return server->getServerName() == name; });
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

void Webserv::logClassData(void) const
{
	std::cout << "\n------------ Webserv Data Log ------------" << std::endl;

	// Log the number of servers
	std::cout << "Number of Servers: " << _servers.size() << std::endl;

	// Log the state of the epoll instance
	std::cout << "Epoll Instance: " << &_epoll << std::endl; // Address of the epoll instance

	// Log the keepRunning flag
	std::cout << "Keep Running Flag: " << _keepRunning.load() << std::endl;

	std::cout << "Webserv() calling the servers" << std::endl;
	for (auto &server : _servers)
	{
		server->logClassData();
	}

	std::cout << "Webserv() calling the Epoll" << std::endl;
	_epoll.logClassData();
}
