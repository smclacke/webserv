/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 17:17:28 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 16:31:50 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "web.hpp"
#include <vector>
#include <string>
#include <iostream>
#include "socket.hpp"

/**
 * @note allowed_methods, GET POST DELETE are all default turned on
 * if 1 is specified in the config file only that one will be turned on;
 */

// std::map<std::string, std::string> serverConfig = {
// 	{"listen", ""},
// 	{"server_name", ""},
// 	{"client_max_body_size", ""},
// 	{"error_page", ""},
// 	{"Location", ""}};

// std::map<std::string, std::string> locationConfig = {
// 	{"allowed_methods", ""},
// 	{"upload_dir", ""},
// 	{"client_max_body_size", ""},
// 	{"root", ""},
// 	{"autoindex", ""},
// 	{"index", ""},
// 	{"cgi_path", ""},
// 	{"cgi_ext", ""}};

// location / files
// {
// 	autoindex on;
// 	allowed_methods GET;
// 	root / var / www / files;
// 	autoindex
// 				upload_dir /
// 			var / uploads -
// 		local;
// }

struct s_location
{
	std::string path;
	std::string root;
	size_t client_body_buffer_size;
	bool allow_GET;
	bool allow_POST;
	bool allow_DELETE;
	bool autoindex;
	std::string upload_dir;
	std::string index;
	std::string cgi_ext;
	std::string cgi_path;
};

struct s_ePage
{
	std::string path;
	int code;
};

class Server
{
private:
	std::string _serverName;
	std::string _host;
	int _port;
	std::vector<s_ePage> _errorPage;
	size_t _clientMaxBodySize; // in megaBytes
	std::vector<s_location> _location;
	Socket _serverSocket;
	Socket _clientSocket;

public:
	Server(void);
	Server &operator=(const Server &rhs);
	Server(std::ifstream server_block);
	~Server(void);
	void printServer(void);

	/* add */
	void addLocation(s_location route);
	void addErrorPage(s_ePage errorPage);

	/* init */
	void initListen(std::string hostPort);

	/* setters */
	void setServerName(std::string serverName);
	void setHost(std::string host);
	void setPort(int port);
	void setErrorPage(std::vector<s_ePage> errorPage);
	void setClientMaxBodySize(size_t clientMaxBodySize);
	void setLocation(std::vector<s_location> location);
	void setServerSocket(Socket serverSocket);
	void setClientSocket(Socket clientSocket);

	/* getters */
	std::string const &getServerName(void) const;
	const std::string &getHost(void) const;
	const int &getPort(void) const;
	const std::vector<s_ePage> &getErrorPage(void) const;
	const size_t &getClientMaxBodySize(void) const;
	const std::vector<s_location> &getLocation(void) const;
	const Socket &getServerSocket(void) const;
	const Socket &getClientSocket(void) const;
};

#endif /* SERVER_HPP */