/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/01 15:50:57 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_HPP
#define WEB_HPP

// CPP includes
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <limits>
#include <cstring>	// might not need
#include <iostream> // for cout
#include <cstdlib>	// for exit() + EXIT_FAILURE
#include <cstdio>

// CPP containers
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <list>
#include <deque>
#include <utility>

// Network includes
#include <poll.h>		//maybe actually C include? - might not need if sys/epoll
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket functions
#include <sys/msg.h>	// check this out (poll)
#include <sys/time.h>	// check this out (poll)
#include <sys/types.h>	// check this out (poll)
#include <sys/epoll.h>
#include <sys/ioctl.h> // for nonblocking stuff..
// # include <sys/select.h> // check this out
// # include <arpa.inet.h> // check this out
#include <arpa/inet.h>
#include <sys/select.h> // for select()

// C includes
#include <unistd.h> // for read
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

/* classes */
#include "webserv.hpp"
#include "server.hpp"
#include "epoll.hpp"
#include "socket.hpp"
#include "error.hpp"

/* enums, defines and errors */
enum class eSocket
{
	Client = 1,
	Server = 2
};

/* parser */
void verifyInput(int ac, char **av);

/* server */


#endif /* WEB_HPP */
