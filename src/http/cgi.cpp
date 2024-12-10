/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 20:53:32 by smclacke      ########   odam.nl         */
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

static void		freeStrings(char *script, char *path)
{
	if (script != NULL)
		free(script);
	if (path != NULL)
		free(path);
}

void httpHandler::cgiResponse()
{

	int		cgiIN[2]; // for sending data to the script
	int		cgiOUT[2]; // for receiving data from the script
	
	if (pipe(cgiIN) < 0 || pipe(cgiOUT) < 0)
	{
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "pip() cgi failed\n";
		return ;
	}
	_response.pid = fork();
	if (_response.pid == 0) // child
	{
		char	*scriptName = strdup(_cgi.scriptname.c_str());
		if (scriptName == NULL)
		{
			closeAllPipes(cgiIN, cgiOUT);
			throw std::runtime_error("failed malloc");
		}
		char	*scriptPath = strdup(_request.path.c_str());
		if (scriptPath == NULL)
		{
			closeAllPipes(cgiIN, cgiOUT);
			freeStrings(scriptName, NULL);
			throw std::runtime_error("failed malloc");
		}

		char	*argv[] = {scriptPath, nullptr};
		
		 // child reads from input pipe + writes to output pipe
		if (dup2(cgiIN[0], STDIN_FILENO) < 0 || dup2(cgiOUT[1], STDOUT_FILENO) < 0)
		{
			std::cerr << "dup2() cgi failed\n";
			closeAllPipes(cgiIN, cgiOUT);
			freeStrings(scriptName, scriptPath);
			exit(EXIT_FAILURE);
		}

		// close unused fds
		protectedClose(cgiIN[1]);
		protectedClose(cgiOUT[0]);

		// execute cgi script
		if (execve(scriptPath, argv, _cgi.env.data()) == -1)
		{
			std::cerr << "execve failed\n";
			freeStrings(scriptName, scriptPath);
			closeAllPipes(cgiIN, cgiOUT);
			exit(EXIT_FAILURE);	
		}

		closeAllPipes(cgiIN, cgiOUT);
		freeStrings(scriptName, scriptPath);
		exit(EXIT_SUCCESS);
	}
	else if (_response.pid > 0) // parent
	{
		/* parent only writes to input and reads from output*/
		// close unused fds
		protectedClose(cgiIN[0]);
		protectedClose(cgiOUT[1]);
	
		// add to monitor
		_epoll.addOUTEpoll(cgiIN[1]);
		_epoll.addToEpoll(cgiOUT[0]);
		
	}
	else // error
	{
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "fork() cgi failed\n";
		return ;
	}
}
