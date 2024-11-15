/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/15 18:24:25 by smclacke      ########   odam.nl         */
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
	_epoll.initEpoll();
	if (config.empty())
	{
		Server default_server;
		_servers.push_back(default_server);

		//Server default_server2(9999);
		//_servers.push_back(default_server2);

		return ;
	}
	else
	{
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
}

Webserv::~Webserv(void)
{
}

/* member functions */
void		Webserv::addServersToEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		t_fds	thisFd;

		thisFd._serverfd = getServer(i).getServerSocket()->getSockfd();
		thisFd._clientfd = getServer(i).getClientSocket()->getSockfd();
		thisFd._serveraddlen = getServer(i).getServerSocket()->getAddrlen();
		thisFd._serveraddr = getServer(i).getServerSocket()->getSockaddr();
		thisFd._event = _epoll.addSocketEpoll(thisFd._serverfd, _epoll.getEpfd(), eSocket::Server);

		_epoll.setFd(thisFd);
	}
}

// will also need to add file(s) to Epoll monitoring
void		Webserv::monitorServers(std::vector<Server> &servers)
{
	while (true)
	{
		for (size_t i = 0; i < getServerCount(); ++i)
		{
			t_fds	thisFd = _epoll.getFd(i);
			_epoll.connectClient(thisFd);
			std::cout << "Client [" << i << "] connected to server successfully\n";
			thisFd._event = _epoll.addSocketEpoll(thisFd._clientfd, _epoll.getEpfd(), eSocket::Client);
			int numEvents = epoll_wait(_epoll.getEpfd(), thisFd._events, 10, -1);
			if (numEvents == -1)
				throw std::runtime_error("epoll_wait() failed\n");

			for (int j = 0; j < numEvents; ++j)
			{
				if (thisFd._events[j].data.fd == thisFd._serverfd)
					_epoll.makeNewConnection(servers[i].getServerSocket(), thisFd);
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

Epoll	&Webserv::getEpoll()
{
	return this->_epoll;
}
