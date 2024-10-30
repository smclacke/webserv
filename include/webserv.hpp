/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 14:54:58 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
	size_t getServerCount(void) const;
	Server &getServer(size_t index);
	Server &getServer(std::string name);
	void start(void);
};

#endif /* WEBSERV_HPP */