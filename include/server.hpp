/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 17:17:28 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/22 17:32:56 by jde-baai      ########   odam.nl         */
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

struct s_route
{
	std::string path;
	size_t client_body_buffer_size;
	bool allow_GET;
	bool allow_POST;
	bool autoindex;
	std::string upload_dir;
	std::string index;
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
	std::vector<s_ePage> _error_page;
	size_t _clientMaxBodySize;
	std::vector<s_route> _routes;

public:
	Server(std::ifstream server_block);
	~Server(void);
	void addRoute(s_route route);
};

#endif /* SERVER_HPP */