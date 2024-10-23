/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/23 12:45:35 by jde-baai      ########   odam.nl         */
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
	Webserv::Webserv(std::string config);
	~Webserv(void);

	/* member functions */
	bool start(void);
};

#endif /* WEBSERV_HPP */