/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 17:17:28 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/23 18:11:17 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <iostream>

/*
	location /files {
		autoindex on;
		allowed_methods GET;
		root /var/www/files;
		autoindex
 upload_dir /var/uploads-local;
	}
*/

struct s_location
{
	std::string path;
	std::string root;
	size_t client_body_buffer_size;
	bool allow_GET;
	bool allow_POST;
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

public:
	Server(void);
	Server(std::ifstream server_block);
	~Server(void);
	void printServer(void);

	/* add */
	void addLocation(s_location route);
	void addErrorPage(s_ePage errorPage);

	/* setters */
	void setServerName(std::string serverName);
	void setHost(std::string host);
	void setPort(int port);
	void setErrorPage(std::vector<s_ePage> errorPage);
	void setClientMaxBodySize(size_t clientMaxBodySize);
	void setLocation(std::vector<s_location> location);

	/* getters */
	std::string getServerName(void) const;
	std::string getHost(void) const;
	int getPort(void) const;
	std::vector<s_ePage> getErrorPage(void) const;
	size_t getClientMaxBodySize(void) const;
	std::vector<s_location> getLocation(void) const;
};

#endif /* SERVER_HPP */