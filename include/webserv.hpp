/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 15:21:17 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <vector>
#include "server.hpp"

class Server;

class Webserv
{
private:
	std::vector<Server> _servers;

public:
	/* constructors */
	Webserv(void);
	Webserv(std::string config);
	~Webserv(void);

	/* member functions */
	void start(void);

	/* Setters */
	void addServer(Server &server);

	/* getters */
	size_t getServerCount(void) const;
	Server &getServer(size_t index);
	Server &getServer(std::string name);
};

#endif /* WEBSERV_HPP */
