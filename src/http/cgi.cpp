/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 17:02:33 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void httpHandler::cgiResponse(std::vector<char *> env)
{
	// need argument array using cgiData struct scriptName
	//char	*argv[] = { // cgiData.scriptName, nullptr};
	char	*argv[] = {nullptr};

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
		// exit child process if execve fails, else?
	}
	else if (_response.pid > 0)
	{
		// handle sending post data to child process if necessary
		
		protectedClose(cgiIN[0]); // parent only write to input
		protectedClose(cgiOUT[1]); // parent only reads from output

		// add to epoll monitoring
		addToEpoll(cgiIN[1]);
		addToEpoll(cgiOUT[0]);
		// events should be added to monitoring and trigger EPOLLOUT/EPOLLIN
		
	}
	else // if forking errors
	{
		closeAllPipes(cgiIN, cgiOUT);
		std::cerr << "forking cgi failed\n";
		return ;
	}
}


/*

write(cgiInput[1], postData.c_str(), postData.length());
close(cgiInput[1]);

// Read output from CGI script
char buffer[1024];
while (read(cgiOutput[0], buffer, sizeof(buffer) - 1) > 0) {
    std::cout << buffer;
}
close(cgiOutput[0]);

Send the Response Back to the Client
After reading the CGI output, send it back as the HTTP response.
*/