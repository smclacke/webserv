/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 12:54:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/23 18:03:37 by jde-baai      ########   odam.nl         */
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
						 "/uploads",
						 "index.html",
						 ".php", "/usr/bin/php-cgi"});
}

Server::Server(std::ifstream server_block)
{
	(void)server_block;
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
		std::cout << "  Autoindex: " << (location.autoindex ? "Yes" : "No") << std::endl;
		std::cout << "  Upload Dir: " << location.upload_dir << std::endl;
		std::cout << "  Index: " << location.index << std::endl;
		std::cout << "  CGI Ext: " << location.cgi_ext << std::endl;
		std::cout << "  CGI Path: " << location.cgi_path << std::endl;
	}
}

/* getters */
std::string Server::getServerName(void) const
{
	return (_serverName);
}