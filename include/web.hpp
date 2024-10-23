/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/23 16:43:28 by jde-baai      ########   odam.nl         */
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
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket functions
// # include <sys/select.h> // check this out
// # include <arpa.inet.h> // check this out

// C includes
#include <unistd.h> // for read
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

/* enums and defines */
enum e_config
{
	DEFAULT = 0,
	SERVER_CONF = 1,
	BAD_INPUT = 2
};

/* classes */
#include "webserv.hpp"
#include "server.hpp"

/* parser */
e_config verifyInput(int ac, char **av);
void initDefault(void);
void initConf(std::string conf);

/* server */
void run(void);

#endif
