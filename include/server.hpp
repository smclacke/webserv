/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 17:17:28 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/29 18:59:49 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "web.hpp"
#include "socket.hpp"

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

const std::unordered_map<eHttpMethod, std::string> HttpMethodToString = {
	{eHttpMethod::GET, "GET"},
	{eHttpMethod::POST, "POST"},
	{eHttpMethod::DELETE, "DELETE"},
	{eHttpMethod::PUT, "PUT"},
	{eHttpMethod::HEAD, "HEAD"},
	{eHttpMethod::OPTIONS, "OPTIONS"},
	{eHttpMethod::PATCH, "PATCH"},
	{eHttpMethod::INVALID, "INVALID"},
};

const std::unordered_map<std::string, eHttpMethod> StringToHttpMethod = {
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
	std::string root = "";												 // Default to standard web root
	size_t client_body_buffer_size = std::numeric_limits<size_t>::max(); // Default buffer size will be set to server max
	std::list<eHttpMethod> allowed_methods;								 // Default: will be set to GET, POST, DELETE in parseLocation if empty
	std::string redir_url = "";											 // No redirection by default
	int redirect_status = 0;											 // No redirection status by default (0 indicates no redirect)
	std::list<std::string> index_files;									 // Standard index files
	bool autoindex = false;												 // Directory listing off by default
	std::string upload_dir = "";										 // Default upload directory
	std::string index = "index.html";									 // Primary index file if directory is requested
	std::string cgi_ext = "";											 // No default CGI extension
	std::string cgi_path = "";											 // No default CGI path
};

struct s_ePage
{
	std::string path;
	int code;
};

class Socket;

class Server
{
	private:
		std::string _serverName;
		in_addr_t _host;
		int _port;
		std::string _root;
		std::vector<s_ePage> _errorPage;
		size_t _clientMaxBodySize; // in Byes (k = * 1024, m = * 1024^2, g = * 1024^3)
		std::vector<s_location> _location;
		std::shared_ptr<Socket> _serverSocket;

	/* Location Parsing */
	void checkLocationPaths(s_location &loc, std::string const root, int const line_n);
	void findLocationDirective(std::string &line, int &line_n, s_location &loc);
	s_location parseLocation(std::ifstream &file, std::string &line, int &line_n, size_t maxbody);
	void parseRoot(std::stringstream &ss, int line_n);

	/* Server parsing*/
	void findServerDirective(Server &serv, std::string &line, int line_n);
	void parseServerName(std::stringstream &ss, int line_n);
	void parseListen(std::stringstream &ss, int line_n);
	void parseErrorPage(std::stringstream &ss, int line_n);
	void parseClientMaxBody(std::stringstream &ss, int line_n);

public:
	Server(void);
	Server &operator=(const Server &rhs);
	Server(std::ifstream &file, int &line_n);
	Server(int portnum);
	~Server(void);

	/* Member functions */
	s_httpSend handleRequest(std::stringstream &request);
	eHttpMethod allowedHttpMethod(std::string &str);
	void printServer(void);

		/* add */
		void addLocation(s_location route);
		void addErrorPage(s_ePage errorPage);

		/* directives parsing */
		
		/* setters */
		void setServerName(std::string serverName);
		void setHost(std::string host);
		void setPort(int port);
		void setRoot(std::string root);
		void setErrorPage(std::vector<s_ePage> errorPage);
		void setClientMaxBodySize(size_t clientMaxBodySize);
		void setLocation(std::vector<s_location> location);
		void setServerSocket(std::shared_ptr<Socket> serverSocket);

		/* getters */
		std::string const 				&getServerName(void) const;
		const in_addr_t 				&getHost(void) const;
		const int 						&getPort(void) const;
		const std::string 				&getRoot(void) const;
		const std::vector<s_ePage> 		&getErrorPage(void) const;
		const size_t					&getClientMaxBodySize(void) const;
		const std::vector<s_location>	&getLocation(void) const;
		std::shared_ptr<Socket>		 	&getServerSocket(void);

};

#endif /* SERVER_HPP */
