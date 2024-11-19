/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:21:02 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 19:44:02 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "server.hpp"
#include "epoll.hpp"

class Server;
class Epoll;

class Webserv
{
	private:
		std::vector<std::shared_ptr<Server>>	_servers;
		Epoll 									_epoll;

	public:
		/* constructors */
		Webserv(void);
		Webserv(std::string config);
		~Webserv(void);

		/* member functions */
		void				addServersToEpoll();
		void				addFilesToEpoll(s_serverData clientSock, std::string file);
		void				monitorServers(std::vector<std::shared_ptr<Server>> &servers);

		/* Setters */
		void				addServer(std::shared_ptr<Server> server);

		/* getters */
		std::shared_ptr<Server> 				getServer(size_t index);
		std::shared_ptr<Server> 				getServer(std::string name);
		std::vector<std::shared_ptr<Server>>	&getAllServers();
		size_t 									getServerCount(void) const;
		Epoll									&getEpoll();
};

#endif /* WEBSERV_HPP */
