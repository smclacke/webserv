/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/15 18:06:11 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "server.hpp"

class Server;

class Webserv
{
private:
	std::vector<std::shared_ptr<Server>> _servers;

public:
	/* constructors */
	Webserv(void);
	Webserv(std::string config);
	~Webserv(void);

	/* member functions */
	void start(void);

	/* Setters */
	void addServer(std::shared_ptr<Server> server);

	/* getters */
	std::shared_ptr<Server> getServer(size_t index);
	std::shared_ptr<Server> getServer(std::string name);
	std::vector<std::shared_ptr<Server>> &getallServer();
	size_t getServerCount(void) const;
};

#endif /* WEBSERV_HPP */
