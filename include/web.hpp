/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/19 15:05:16 by jde-baai      ########   odam.nl         */
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

#define READ_BUFFER_SIZE 100
#define WRITE_BUFFER_SIZE 100

/* http */
/**
 * @param msg the httpResponse message to be send to the client
 * @param keepAlive true if connection should stay alive, false is it should close
 * @param readFile true if there is a fileDescriptor that should be read
 * @param readFd the FD to read from
 * @param cgi true if its a cgi request, call getCGI for cgi data
 */
struct s_httpSend
{
	std::string msg;
	bool keepAlive;
	bool readfile;
	int readFd;
	bool cgi;
	void clearHttpSend(void);
};

enum class cgiState
{
	BEGIN = 0,
	READING = 1,
	WRITING = 2,
	READY = 3,
	ERROR = 4,
	CLOSE = 5
};

struct s_cgi
{
	std::vector<char *> env;
	std::string scriptname;
	int cgiIN[2];  // for sending data to the script
	int cgiOUT[2]; // for receiving data from the script
	enum cgiState state;
	bool complete;
	std::string input;
	size_t write_offset;
	pid_t pid = -1;
	int client_fd;
	std::string output;
	bool httpOutput; // defines if the output should be http formatted

	void clearCgi(void);
	void closeAllPipes(void);
};

/* parser */
void lineStrip(std::string &line);
void verifyInput(int ac, char **av);
s_location addDefaultLoc(size_t servermaxsize);

/* general utils */
void protectedClose(int fd);

#endif /* WEB_HPP */
