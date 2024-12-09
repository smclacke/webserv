/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/09 18:52:19 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

/*
	- args[0] has path of cgi program or script e.g. "/usr/local/bin/php-cgi"
	- args[1] hsa path of cgi file, if script file == NULL
	- args[2] == NULL
	- env has parsed request and variables accoring to RFC(3875) - num pos spec cgi
*/

/**
 * check if file is executable
 * pipe
 * fork
 * in child:
 * 	-close read end
 * 	-Set paramaters
 * 	-execve
 * else
 * 	-close write end
 * 	- set CGI as true
 * 	- set ReadFD
 * 	- set pid
 */


 struct s_cgi
{
	int		pipes[2];
	char	*tmp_file; // what | where are you?
	int		fd;
	int		pid;
	char	*args[3];
	char	**env;
		
};

void httpHandler::cgiResponse(void)
{
	std::cout << "It is a CGI request" << std::endl;
	
	s_cgi	cgi;
	cgi.args[0] = NULL;
	cgi.args[1] = NULL;
	cgi.args[2] = NULL;
	
	cgi.fd = open(cgi.tmp_file, O_CREAT | O_RDWR);
	if (cgi.fd == -1)
	{
		std::cerr << "uable to open CGI file\n";
		return ;	
	}
	if (pipe(cgi.pipes) == -1)
	{
		std::cerr << "Pipe() failed for cgi process\n";
		protectedClose(cgi.fd);
		return ;
	}
	if ((cgi.pid = fork()) == 0)
	{
		protectedClose(cgi.pipes[0]); // close read
		// set params
		execve(cgi.args[0], cgi.args, cgi.env);
	}
	else
	{
		protectedClose(cgi.pipes[1]); // close write
		_response.cgi = true;
		// set ReadFd
		// set pid
	}

	return;
}