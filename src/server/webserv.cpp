/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:22:59 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/24 15:19:12 by smclacke      ########   odam.nl         */
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

static void		printSocketAddress(struct sockaddr_in *addr)
{
	char	ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN);
	std::cout << "Socket Address = " << ipStr << " : " << ntohs(addr->sin_port) << " \n\n";
}

/* member functions */
void		Webserv::addServersToEpoll()
{
	std::cout << "Adding servers to Epoll...\n";
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::shared_ptr<Server>		currentServer = getServer(i);
		t_serverData				thisServer;
		
		thisServer._server = currentServer;

		int					serverSockfd = currentServer->getServerSocket()->getSockfd();
		struct epoll_event 	event;
		event.data.fd = serverSockfd;
		_epoll.addSocketEpoll(serverSockfd, _epoll.getEpfd(), eSocket::Server);
		_epoll.setEvent(event);
		std::cout << "Added server socket to epoll\n";

		int					clientSockfd = currentServer->getClientSocket()->getSockfd();
		event.data.fd = clientSockfd;
		_epoll.addSocketEpoll(clientSockfd, _epoll.getEpfd(), eSocket::Client);
		_epoll.setEvent(event);

		_epoll.setServer(thisServer);
		_epoll.connectClient(thisServer);
		std::cout << "Added client socket to epoll\n";
		struct sockaddr_in addr = currentServer->getServerSocket()->getSockaddr();
		printSocketAddress(&addr);
	}
	std::cout << "--------------------------\n";
}

void		Webserv::addFilesToEpoll(s_serverData clientSock, std::string file)
{
	(void) clientSock;
	(void) file;
	//int		fileFd = open(file.c_str(), O_RDONLY);
	
	//if (fileFd == -1)
	//	throw std::runtime_error("Failed to open file\n");
	//_epoll.addToEpoll(fileFd, _epoll.getEpfd(), clientSock._event);
}

// the socket addresses are the same for server 8080 and server 9999.. this isnt good
// also, after creating sockets, their addresses shouldnt be 0.0.0.0 but 127.0.0.1 but need this is correct format
void		Webserv::printAllServerSocketEpoll()
{
	for (size_t i = 0; i < getServerCount(); ++i)
	{
		std::cout << "epoll epfd = " << _epoll.getEpfd() << " \n\n";
		t_serverData	server = _epoll.getServer(i);
		std::cout << "server socket fd = " << server._server->getServerSocket()->getSockfd() << " \n";
		std::cout << "client socket fd = " << server._server->getClientSocket()->getSockfd() << " \n\n";
		
		sockaddr_in serverSockAddr = server._server->getServerSocket()->getSockaddr();
		socklen_t	servAddrlen = sizeof(serverSockAddr);
		std::cout << "server socket addr = " << &serverSockAddr << " | len = " << servAddrlen << " \n";
		sockaddr_in clientSockAddr = server._server->getClientSocket()->getSockaddr();
		socklen_t	cliAddrlen = sizeof(clientSockAddr);
		std::cout << "client socket addr = " << &clientSockAddr << " | len = " << cliAddrlen << " \n\n";
	}

	//exit(EXIT_SUCCESS);
}

void		Webserv::monitorServers(std::vector<std::shared_ptr<Server>> &servers)
{
	(void) servers;
	std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~\n";
	std::cout << "Entering monitoring loop\n";
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~\n";

	while (true)
	{
		// all servers already added to epoll, can just call wait on all of them
		int numEvents = epoll_wait(_epoll.getEpfd(), _epoll.getAllEvents().data(), 10, TIMEOUT);
		if (numEvents == -1)
			throw std::runtime_error("epoll_wait() failed\n");
		else if (numEvents == 0)
			continue ;

		std::cout << "num events = " << numEvents << " \n";

		// process events returned by epoll_wait
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = _epoll.getAllEvents()[i].data.fd;
			std::cout << "events fd = " << fd << " \n";
			//bool handled = false;

			// check if event is for server socket (new connection)
			for (size_t j = 0; j < getServerCount(); ++j)
			{
				t_serverData thisServer = _epoll.getServer(j);
				std::cout << "server = " << thisServer._server->getServerSocket()->getSockfd() << " \n";
				if (fd == thisServer._server->getServerSocket()->getSockfd())
				{
					std::cout << "make new connection\n";
					_epoll.makeNewConnection(fd, thisServer);
					_epoll.modifyEvent(fd, _epoll.getEpfd(), EPOLLIN);
					break ;
					//handled = true;
				}
			}
			//exit(EXIT_SUCCESS);
			////if (!handled) // event is not for server socket, must be client socket, handle read/write
			////{
			//	for (size_t j = 0; j < getServerCount(); ++j)
			//	{
			//		t_serverData thisServer = _epoll.getServer(j);

			//		_epoll.clientTime(thisServer);
			//		if (_epoll.getAllEvents()[i].events & EPOLLIN)
			//		{
			//			std::cout << "EPOLLIN\n";
			//			if (_epoll.handleRead(thisServer, i))
			//				_epoll.modifyEvent(fd, _epoll.getEpfd(), EPOLLOUT);
			//		}
			//		else if (_epoll.getAllEvents()[i].events & EPOLLOUT)
			//		{
			//			std::cout << "EPOLLOUT\n";
			//			if (_epoll.handleWrite(thisServer, i))
			//				_epoll.modifyEvent(fd, _epoll.getEpfd(), EPOLLIN);
			//		}
			//		else if (_epoll.getAllEvents()[i].events & EPOLLHUP)
			//		{
			//			std::cout << "EPOLLHUP\n";
			//			return ;
			//		}
			//			//_epoll.handleClose(thisServer, i)
			//	}
			//}
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
