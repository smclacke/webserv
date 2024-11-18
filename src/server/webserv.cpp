/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 15:28:29 by smclacke      ########   odam.nl         */
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
	_epoll.initEpoll();
    if (config.empty())
    {
        auto default_server = std::make_shared<Server>(8080);
        _servers.push_back(default_server);
        auto default_server2 = std::make_shared<Server>(9999);
        _servers.push_back(default_server2);
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
void		Webserv::addServersToEpoll()
{
	std::cout << "Adding servers to Epoll...\n";
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		t_fds	thisFd;

		thisFd._serverfd = getServer(i)->getServerSocket()->getSockfd();
		thisFd._serveraddlen = getServer(i)->getServerSocket()->getAddrlen();
		thisFd._serveraddr = getServer(i)->getServerSocket()->getSockaddr();
		thisFd._event = _epoll.addSocketEpoll(thisFd._serverfd, _epoll.getEpfd(), eSocket::Server);

		_epoll.setFd(thisFd);
		std::cout << "Added server [" << i << "] to epoll monitoring\n";
	}
	std::cout << "--------------------------\n";
}

// will also need to add file(s) to Epoll monitoring
void		Webserv::monitorServers(std::vector<std::shared_ptr<Server>> &servers)
{
	std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~\n";
	std::cout << "Entering monitoring loop\n";
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~\n\n";

	while (true)
	{
		for (size_t i = 0; i < getServerCount(); ++i)
		{
			t_fds	thisFd = _epoll.getFd(i);
			
			// do i need to connect client here?
			_epoll.connectClient(thisFd);
			// does this need to be added to epoll here too?
			// does client need to be a socket already or shall i just create server sockets and then accept new connections?
		
			int numEvents = epoll_wait(_epoll.getEpfd(), thisFd._events, 10, -1);
			if (numEvents == -1)
				throw std::runtime_error("epoll_wait() failed\n");

			for (int j = 0; j < numEvents; ++j)
			{
				if (thisFd._events[j].data.fd == thisFd._serverfd)
					_epoll.makeNewConnection(servers[i]->getServerSocket(), thisFd);
				// check here connection timeout, delete 
				else if (thisFd._events[j].events & EPOLLIN)
					_epoll.readIncomingMessage(thisFd, j);
				else if (thisFd._events[j].events & EPOLLOUT)
					_epoll.sendOutgoingResponse(thisFd, j);
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

Epoll	&Webserv::getEpoll()
{
	return this->_epoll;
}
