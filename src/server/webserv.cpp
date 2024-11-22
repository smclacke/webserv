/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/22 15:28:14 by smclacke      ########   odam.nl         */
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
	_epoll.initEpoll();
    if (config.empty())
    {
		std::cout << "default configuration\n";
        auto default_server = std::make_shared<Server>(8080);
        _servers.push_back(default_server);
        auto default_server2 = std::make_shared<Server>(9999);
        _servers.push_back(default_server2);
        return;
    }
    std::cout << "config: " << config << std::endl;
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
void		Webserv::addServersToEpoll()
{
	std::cout << "Adding servers to Epoll...\n";
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		t_serverData	thisServer;

		thisServer._serverSock = getServer(i)->getServerSocket()->getSockfd();
		thisServer._clientSock = getServer(i)->getClientSocket()->getSockfd();
		thisServer._serverAddlen = getServer(i)->getServerSocket()->getAddrlen();
		thisServer._serverAddr = getServer(i)->getServerSocket()->getSockaddr();
		thisServer._event = _epoll.addSocketEpoll(thisServer._serverSock, _epoll.getEpfd(), eSocket::Server);

		_epoll.setServer(thisServer);
		_epoll.connectClient(thisServer);
		std::cout << "Added server [" << i << "] sockets to epoll monitoring\n";
	}
	std::cout << "--------------------------\n";
}

void		Webserv::addFilesToEpoll(s_serverData clientSock, std::string file)
{
	int		fileFd = open(file.c_str(), O_RDONLY);
	
	if (fileFd == -1)
		throw std::runtime_error("Failed to open file\n");
	_epoll.addToEpoll(fileFd, _epoll.getEpfd(), clientSock._event);
}

// STOP USING i WHEN I MEAN TO USE j
// am i even using the client socket?
void		Webserv::monitorServers(std::vector<std::shared_ptr<Server>> &servers)
{
	std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~\n";
	std::cout << "Entering monitoring loop\n";
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~\n";

	while (true)
	{
		for (size_t i = 0; i < getServerCount(); ++i)
		{
			if (_epoll.getServer(i)._serverSock < 0)
				throw std::runtime_error("Server fd is < 0\n");
			t_serverData	thisServer = _epoll.getServer(i);
			_epoll.clientTime(thisServer);

			int numEvents = epoll_wait(_epoll.getEpfd(), thisServer._events, 10, TIMEOUT);
			if (numEvents == -1)
				throw std::runtime_error("epoll_wait() failed\n");
			else if (numEvents == 0)
				continue ;

			for (int j = 0; j < numEvents; ++j)
			{
				if (thisServer._events[j].data.fd == thisServer._serverSock)
					_epoll.makeNewConnection(servers[i]->getServerSocket(), thisServer);
				else if (thisServer._events[j].events & EPOLLIN)
				{
					std::cout << "read\n";
					_epoll.handleRead(thisServer, j);

				}
				else if (thisServer._events[j].events & EPOLLOUT)
				{
					std::cout << "write\n";
					_epoll.handleWrite(thisServer, j);
				}
				else if (EPOLLHUP)
				{
					std::cout << "hup\n";
					// hang up happened on fd
					// handle close connection
					std::cout << "EPOLLHUP -> breaking\n";
					break ;
				}
			}
		}
	}
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
							   return server->getServerName() == name; // Assuming you have a method to get the server name
						   });
	if (it != _servers.end())
	{
		return *it; // Return the found server
	}
	throw std::runtime_error("Server not found"); // Handle the case where the server is not found
}

Epoll	&Webserv::getEpoll()
{
	return this->_epoll;
}
