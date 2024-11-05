/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 17:17:28 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/05 13:22:03 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "web.hpp"
#include <vector>
#include <string>
#include <iostream>
#include "socket.hpp"

enum class eSocket;

enum class eHttpMethod
{
	GET,
	POST,
	DELETE,
	PUT,
	HEAD,
	OPTIONS,
	PATCH,
	INVALID
};

const std::map<eHttpMethod, std::string> HttpMethodToString = {
	{eHttpMethod::GET, "GET"},
	{eHttpMethod::POST, "POST"},
	{eHttpMethod::DELETE, "DELETE"},
	{eHttpMethod::PUT, "PUT"},
	{eHttpMethod::HEAD, "HEAD"},
	{eHttpMethod::OPTIONS, "OPTIONS"},
	{eHttpMethod::PATCH, "PATCH"},
};

const std::map<std::string, eHttpMethod> StringToHttpMethod = {
	{"GET", eHttpMethod::GET},
	{"POST", eHttpMethod::POST},
	{"DELETE", eHttpMethod::DELETE},
	{"PUT", eHttpMethod::PUT},
	{"HEAD", eHttpMethod::HEAD},
	{"OPTIONS", eHttpMethod::OPTIONS},
	{"PATCH", eHttpMethod::PATCH},
};

struct s_location
{
	std::string path = "/";
	std::string root = "/var/www/html";		 // Default to standard web root
	size_t client_body_buffer_size = 8192;	 // Default buffer size, 8 KB
	std::list<eHttpMethod> allowed_methods;	 // Default: will be set to GET, POST, DELETE in parseLocation if empty
	std::string redir_url = "";				 // No redirection by default
	int redirect_status = 0;				 // No redirection status by default (0 indicates no redirect)
	std::list<std::string> index_files;		 // Standard index files
	bool autoindex = false;					 // Directory listing off by default
	std::string upload_dir = "/tmp/uploads"; // Default upload directory
	std::string index = "index.html";		 // Primary index file if directory is requested
	std::string cgi_ext = "";				 // No default CGI extension
	std::string cgi_path = "";				 // No default CGI path
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
	size_t _clientMaxBodySize; // in Byes (k = * 1024, m = * 1024^2, g = * 1024^3)
	std::vector<s_location> _location;
	Socket _serverSocket;
	Socket _clientSocket;

public:
	Server(void);
	Server &operator=(const Server &rhs);
	Server(std::ifstream &file, int &line_n);
	~Server(void);

	/* Member functions */
	eHttpMethod allowedHttpMethod(std::string &str);
	void printServer(void);

	/* add */
	void addLocation(s_location route);
	void addErrorPage(s_ePage errorPage);

	/* directives */
	void parseServerName(std::stringstream &ss, int line_n);
	void parseListen(std::stringstream &ss, int line_n);
	void parseErrorPage(std::stringstream &ss, int line_n);
	void parseClientMaxBody(std::stringstream &ss, int line_n);

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