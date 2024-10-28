/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   run.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:25:39 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/28 15:58:58 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/**
 * CLASS WEBSERV -
 * 
 * 	std::vector<Server>	_servers
 * 	Webserv(std::string config)
 * 
 * getserver(size_t index)
 * getserver(std::string name)
 * 
 * 
 * CLASS SERVER -
 * 
 * 	std::string _serverName
 * 	std::string _host
 * 	int		_port
 * // error page stuff
 * size_t	_clientMaxBodySize
 * std::vector<s_locaiton> 	_location
 * 
 * // setters inc: servername, host, pot, errorpage, clientmaxbodysize, location
 * Server(std::ifstream server_block);
 * 
 * 
 * STRUCT LOCATION -
 * 
 * 	std::string path
 * 	std::stirng root
 * 	size_t client_body_buffer_size
 * 	bools  = allow_GET allow_POST autoindex
 * std::string upload_dir
 * std::string index
 * std::string cgi_ext
 * std::string cgi_path
 * 
 * 
 * CONFIG EXAMPLE:
 * 	
 * 
 * 	location /files
 * 	autoindex on
 * 	alowed_methods GET
 * 	root /var/www/files
 * 	autoindex
 * 	upload_dir /var/uploads-local 
 * 
 */

void run(void)
{
	Webserv			servers("input"); // will be passed to run from main
	// Webserv instance <servers> -> std::vector<Server> _servers: 

	Socket	sock(servers);

	// poll();

	sock.closeSockets();
}
