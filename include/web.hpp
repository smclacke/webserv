/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/08 17:10:18 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_HPP
#define WEB_HPP

/** @todo check what of you we can get rid of */
// CPP includes
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <limits>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>	  // shared pointers header
#include <filesystem> // for directory checking
#include <optional>
#include <atomic>

// CPP containers
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <list>
#include <deque>
#include <utility>
#include <unordered_map>
#include <unordered_set>

// Network includes
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket functions
#include <sys/epoll.h>
#include <sys/ioctl.h> // for nonblocking stuff..
#include <arpa/inet.h>
#include <sys/select.h> // for select()
#include <sys/types.h>
#include <sys/wait.h>

// C includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <csignal>

/* forward declarations*/
struct s_location;
class Server;
class Webserv;

/* enums, defines and errors */
enum class eSocket
{
	Client = 1,
	Server = 2
};

/* http */
struct s_httpSend
{
	std::string		msg;
	bool			keepAlive;
	std::string		filepath;
	bool			readfile;
	int				readFd;
	pid_t			pid;
};

/* parser */
void			lineStrip(std::string &line);
void			verifyInput(int ac, char **av);
s_location		parseLocation(std::ifstream &file, std::string &line, int &line_n, size_t maxbody);
s_location		addDefaultLoc(size_t servermaxsize);

/* utils */
void			protectedClose(int fd);

#endif /* WEB_HPP */
