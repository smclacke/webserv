/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 17:11:31 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void httpHandler::cgiResponse(std::vector<char *> env)
{
	// need argument array using cgiData struct scriptName
	char	*argv[] = {_cgi.scriptName, nullptr};

	int		cgiIN[2]; // for sending data to the script
	int		cgiOUT[2]; // for receiving data from the script
	
	if (pipe(cgiIN) < 0)
	{
		std::cerr << "piping cgi failed\n";
		return ;
	}
	if (pipe(cgiOUT) < 0)
	{
		protectedClose(cgiIN[0]);
		protectedClose(cgiIN[1]);
		std::cerr << "piping cgi failed\n";
		return ;
	}
	_response.pid = fork();
	if (_response.pid == 0)
	{
		if (dup2(cgiIN[0], STDIN_FILENO) < 0) // child reads from input pipe
		{
			closeAllPipes(cgiIN, cgiOUT);
			std::cerr << "duping input read end failed\n";
			return ;
		}
		if (dup2(cgiOUT[1], STDOUT_FILENO)); // child writes to output pipe
		{
			closeAllPipes(cgiIN, cgiOUT);
			std::cerr << "duping output write end failed\n";
			return ;
		}
		protectedClose(cgiIN[1]);
		protectedClose(cgiOUT[0]);

		// execute python script
		if (execve(_request.path.c_str(), argv, env.data()) == -1)
		{
			std::cerr << "execve failed\n";
			closeAllPipes(cgiIN, cgiOUT);
			exit(-1);	
		} 
		exit(0); // successfully exit child process
	}
	else if (_response.pid > 0)
	{
		// handle sending post data to child process if necessary
		
		protectedClose(cgiIN[0]); // parent only write to input
		protectedClose(cgiOUT[1]); // parent only reads from output

		// add to epoll monitoring
		addToEpoll(cgiIN[1]);
		addToEpoll(cgiOUT[0]);

	}
	else // if forking errors
	{
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "forking cgi failed\n";
		return ;
	}
}
