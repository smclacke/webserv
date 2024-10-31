/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 12:54:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/31 12:38:42 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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
						 false,
						 true,
						 true,
						 true,
						 true,
						 "/uploads",
						 "index.html",
						 ".php", "/usr/bin/php-cgi"});
	Socket _clientSocket(eSocket::Client);
	Socket _serverSocket(eSocket::Server);
}

Server &Server::operator=(const Server &rhs)
{
	if (this != &rhs)
	{
		_serverName = rhs._serverName;
		_host = rhs._host;
		_port = rhs._port;
		_errorPage = rhs._errorPage;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_location = rhs._location;
	}
	return *this;
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

/* directives */

void Server::parseServerName(std::stringstream &ss, int line_n)
{
	std::string name;
	std::string unexpected;
	ss >> name;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	setServerName(name);
}

void Server::parseListen(std::stringstream &ss, int line_n)
{
	std::string value;
	std::string unexpected;
	ss >> value;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	size_t colonPos = value.find(':');
	if (colonPos == std::string::npos)
		throw eConf("Invalid listen directive: missing \':\'", line_n);
	std::string host = value.substr(0, colonPos);
	std::string portStr = value.substr(colonPos + 1);

	// checking the host IP for validity
	std::istringstream ipStream(host);
	std::string part;
	int partCount = 0;
	while (std::getline(ipStream, part, '.'))
	{
		if (part.empty() || part.length() > 3 || !std::all_of(part.begin(), part.end(), ::isdigit))
			throw eConf("Invalid host format. Expected 0.0.0.0", line_n);
		int num = std::stoi(part);
		if (num < 0 || num > 255)
			throw eConf("Invalid host format. Each part must be between 0 and 255", line_n);
		partCount++;
	}
	if (partCount != 4)
		throw eConf("Invalid host format. Expected 4 parts", line_n);
	setHost(host);
	// checking the port number for validity
	if (portStr.length() != 4 || !std::all_of(portStr.begin(), portStr.end(), ::isdigit))
		throw eConf("Invalid port format. Expected 4 digits", line_n);
	setPort(std::stoi(portStr));
}

void Server::parseErrorPage(std::stringstream &ss, int line_n)
{
	(void)ss;
	(void)line_n;
}

void Server::parseClientMaxBody(std::stringstream &ss, int line_n)
{
	(void)ss;
	(void)line_n;
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

void Server::setServerSocket(Socket serverSocket)
{
	_serverSocket = serverSocket;
}

void Server::setClientSocket(Socket clientSocket)
{
	_clientSocket = clientSocket;
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

const Socket &Server::getServerSocket(void) const
{
	return _serverSocket;
}

const Socket &Server::getClientSocket(void) const
{
	return _clientSocket;
}