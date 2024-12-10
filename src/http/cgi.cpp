/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 18:06:23 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"


static void		closeAllPipes(int cgiIN[2], int cgiOUT[2])
{
	if (cgiIN[0])
		protectedClose(cgiIN[0]);
	if (cgiIN[1])
		protectedClose(cgiIN[1]);
	if (cgiOUT[0])
		protectedClose(cgiOUT[0]);
	if (cgiOUT[1])
		protectedClose(cgiOUT[1]);
}

/** @todo -> get epoll in here correctly
 * -> get everything in cgi struct and use that here
 * -> make sure we dont have multiple **envs or anything else unncessary
 * -> make sure the path being sent in execve is absolute path to the executable/script
 * -> check getting the scriptname into cgi struct is ok
*/
void httpHandler::cgiResponse(std::vector<char *> env)
{
	// need argument array using cgiData struct scriptName
	char	*argv[] = {_cgi.scriptName, nullptr};

	int		cgiIN[2]; // for sending data to the script
	int		cgiOUT[2]; // for receiving data from the script
	
	if (pipe(cgiIN) < 0 || pipe(cgiOUT) < 0)
	{
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "pip() cgi failed\n";
		return ;
	}

	_response.pid = fork();
	if (_response.pid == 0)
	{
		 // child reads from input pipe + writes to output pipe
		if (dup2(cgiIN[0], STDIN_FILENO) < 0 || dup2(cgiOUT[1], STDOUT_FILENO) < 0)
		{
			closeAllPipes(cgiIN, cgiOUT);
			std::cerr << "dup2() cgi failed\n";
			exit(EXIT_FAILURE);
		}

		// close unused fds
		protectedClose(cgiIN[1]);
		protectedClose(cgiOUT[0]);

		// execute cgi script
		if (execve(_request.path.c_str(), argv, env.data()) == -1)
		{
			std::cerr << "execve failed\n";
			closeAllPipes(cgiIN, cgiOUT);
			exit(EXIT_FAILURE);	
		} 
	}
	else if (_response.pid > 0)
	{
		// close unused fds
		protectedClose(cgiIN[0]); // parent only write to input
		protectedClose(cgiOUT[1]); // parent only reads from output
		
		// add to epoll, monitor I/O events
		_epoll.addPipeEpoll(cgiIN[1]);
		_epoll.addPipeEpoll(cgiOUT[0]);
	}
	else
	{
		// fork errored
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "fork() cgi failed\n";
		return ;
	}
}
