/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/05 13:12:50 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include "server.hpp"
#include "epoll.hpp"

class Server;
class Epoll;

class Webserv
{
private:
	std::vector<Server> _servers;
	Epoll 				_epoll;

public:
	/* constructors */
	Webserv(void);
	Webserv(std::string config);
	~Webserv(void);

	/* member functions */

	/* Setters */
	void addServer(Server &server);

	/* getters */
	std::vector<Server> &getallServer();
	size_t getServerCount(void) const;
	Server &getServer(size_t index);
	Server &getServer(std::string name);
};

#endif /* WEBSERV_HPP */
