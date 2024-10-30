/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 12:54:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 14:59:27 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/**
 * @brief	data filler for testing
 * @note	to be removed.
 */
Server::Server(void)
{
	_serverName = "default_server";
	_host = "127.0.0.01";
	_port = 8080;
	_errorPage.push_back({"/404.html", 404});
	_clientMaxBodySize = 10;
	_location.push_back({"/",
						 "root",
						 1,
						 true,
						 true,
						 true,
						 true,
						 "/uploads",
						 "index.html",
						 ".php", "/usr/bin/php-cgi"});
}

Server::Server(std::ifstream server_block)
{
	(void)server_block;
	std::string line;
	while (std::getline(server_block, line))
	{
		if (line.find("}") != std::string::npos)
			break;
		// keymap
	}
}

Server::~Server()
{
}

void Server::printServer(void)
{
	std::cout << "Server Name: " << _serverName << std::endl;
	std::cout << "Host: " << _host << std::endl;
	std::cout << "Port: " << _port << std::endl;
	std::cout << "Client Max Body Size: " << _clientMaxBodySize << "MB" << std::endl;
	std::cout << "Error Pages:" << _errorPage.size() << std::endl;
	for (const auto &errorPage : _errorPage)
	{
		std::cout << "  Path: " << errorPage.path << ", Code: " << errorPage.code << std::endl;
	}
	std::cout << "Locations:" << _location.size() << std::endl;
	for (const auto &location : _location)
	{
		std::cout << "  Path: " << location.path << std::endl;
		std::cout << "  Root: " << location.root << std::endl;
		std::cout << "  Client Body Buffer Size: " << location.client_body_buffer_size << std::endl;
		std::cout << "  Allow GET: " << (location.allow_GET ? "Yes" : "No") << std::endl;
		std::cout << "  Allow POST: " << (location.allow_POST ? "Yes" : "No") << std::endl;
		std::cout << "  Allow DELETE: " << (location.allow_DELETE ? "Yes" : "No") << std::endl;
		std::cout << "  Autoindex: " << (location.autoindex ? "Yes" : "No") << std::endl;
		std::cout << "  Upload Dir: " << location.upload_dir << std::endl;
		std::cout << "  Index: " << location.index << std::endl;
		std::cout << "  CGI Ext: " << location.cgi_ext << std::endl;
		std::cout << "  CGI Path: " << location.cgi_path << std::endl;
	}
}

/* setters */

void Server::addLocation(s_location route)
{
	_location.push_back(route);
}

void Server::addErrorPage(s_ePage errorPage)
{
	_errorPage.push_back(errorPage);
}

void Server::initListen(std::string hostPort)
{
	hostPort.erase(hostPort.find_first_not_of(" \t\n\r\f\v") + 1);
	hostPort.erase(hostPort.find_last_not_of(" \t\n\r\f\v;") + 1);
	std::istringstream ss(hostPort);
	std::string host, port;
	std::getline(ss, host, ':');
	std::getline(ss, port);
	_host = host;
	_port = std::stoi(port);
}

void Server::setServerName(std::string serverName)
{
	_serverName = serverName;
}

void Server::setHost(std::string host)
{
	_host = host;
}

void Server::setPort(int port)
{
	_port = port;
}

void Server::setErrorPage(std::vector<s_ePage> errorPage)
{
	_errorPage = errorPage;
}

void Server::setClientMaxBodySize(size_t clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

void Server::setLocation(std::vector<s_location> location)
{
	_location = location;
}

/* getters */
const std::string &Server::getServerName(void) const
{
	return _serverName;
}

const std::string &Server::getHost(void) const
{
	return _host;
}

const int &Server::getPort(void) const
{
	return _port;
}

const std::vector<s_ePage> &Server::getErrorPage(void) const
{
	return _errorPage;
}

const size_t &Server::getClientMaxBodySize(void) const
{
	return _clientMaxBodySize;
}

const std::vector<s_location> &Server::getLocation(void) const
{
	return _location;
}
