/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/12 20:04:39 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

static void		freeStrings(char *script, char *path)
{
	if (script != NULL)
		free(script);
	if (path != NULL)
		free(path);
}

void	Epoll::handleCgiRead(s_cgi &cgi)
{
	std::cout << "in read\n";
	char	buffer[READ_BUFFER_SIZE];
	int		bytesRead;

	memset(buffer, 0, sizeof(buffer));
	bytesRead = read(cgi.cgiOUT[0], buffer, READ_BUFFER_SIZE - 1);
	if (bytesRead < 0)
	{
		send(cgi.client_fd, BAD_CGI, BAD_SIZE, 0);
		std::cerr << "read() cgi failed\n"; /** @todo remove after testing*/
		cgi.close = true;
		return ;
	}
	else if (bytesRead == 0) // EOF
	{
		std::cout << "finished reading\n";
		// GIVE THIS TO HTTPRESPONSE
		send(cgi.client_fd, cgi.input.c_str(), cgi.input.size(), 0);
		close(cgi.cgiOUT[0]);
		cgi.cgiOUT[0] = -1;
		return ;
	}
	cgi.output = true;
	cgi.input.append(buffer, bytesRead);
	if (bytesRead <= READ_BUFFER_SIZE)
	{
		std::cout << "finished reading, no more to read\n";
		// GIVE THIS TO HTTPRESPONSE
		close(cgi.cgiOUT[0]);
		cgi.cgiOUT[0] = -1;
		return ;
		
	}
	std::cout << "cgi READ input = " << cgi.input << "\n\n";
	return ;
}

void	Epoll::handleCgiWrite(s_cgi &cgi)
{
	if (cgi.input.size() == 0) //  we need to read
	{
		close(cgi.cgiIN[1]);
		cgi.close = false;
		return ;
	}

	std::cout << "CLIENT FD = " << cgi.client_fd << "\n\n";

	std::cout << "cgi WRITE input size = " << cgi.input.size() << "\n\n";
	std::cout << "cgi WRITE input = " << cgi.input << "\n\n";
	//int bytesWritten = send(cgi.client_fd, cgi.input.c_str(), cgi.input.size(), 0);
	
	//modifyEvent(cgi.client_fd, EPOLLOUT);
	int bytesWritten = write(cgi.client_fd, cgi.input.c_str(), cgi.input.size());
	std::cout << "cgi WRITE byteswritten = " << bytesWritten << "\n\n";
	
	if (bytesWritten < 0)
	{
		std::cerr << "write() to cgi failed\n"; /** @todo remove after testing*/
		cgi.close = true;
		return ;
	}
	//if (!cgi.input.empty())
	//{
		
	//}
	//	modifyEvent(cgi.client_fd, EPOLLOUT);
	
	if (cgi.input.empty() && cgi.cgiOUT[0] == -1) // everything has been sent
	{
		cgi.close = true;
		return ;
	}
	cgi.state = cgiState::BEGIN;
	cgi.close = false;
}

void		httpHandler::cgiResponse()
{
	if (pipe(_cgi.cgiIN) < 0 || pipe(_cgi.cgiOUT) < 0)
	{
		_cgi.closeAllPipes();
		setErrorResponse(eHttpStatusCode::InternalServerError, "failed to open pipes");
		return ;
	}
	_cgi.pid = fork();
	if (_cgi.pid == 0) // child: reads from input pipe + writes to output pipe
	{
		char	*scriptName = strdup(_cgi.scriptname.c_str());
		if (scriptName == NULL)
		{
			_cgi.closeAllPipes();
			std::exit(EXIT_FAILURE);
		}
		char	*scriptPath = strdup(_request.path.c_str());
		if (scriptPath == NULL)
		{
			_cgi.closeAllPipes();
			freeStrings(scriptName, NULL);
			std::exit(EXIT_FAILURE);
		}
		char	*argv[] = {scriptPath, nullptr};
		
		if (dup2(_cgi.cgiIN[0], STDIN_FILENO) < 0 || dup2(_cgi.cgiOUT[1], STDOUT_FILENO) < 0)
		{
			std::cerr << "dup2() cgi failed\n";  /** @todo remove after testing*/
			_cgi.closeAllPipes();
			freeStrings(scriptName, scriptPath);
			std::exit(EXIT_FAILURE);
		}
		protectedClose(_cgi.cgiIN[1]);
		protectedClose(_cgi.cgiOUT[0]);
		if (execve(scriptPath, argv, _cgi.env.data()) == -1)
		{
			std::cerr << "execve failed\n"; /** @todo remove after testing*/
			freeStrings(scriptName, scriptPath);
			_cgi.closeAllPipes();
			std::exit(EXIT_FAILURE);	
		}
		_cgi.closeAllPipes();
		freeStrings(scriptName, scriptPath);
		std::exit(EXIT_SUCCESS);
	}
	else if (_cgi.pid > 0) // parent: only writes to input and reads from output
	{
		protectedClose(_cgi.cgiIN[0]);
		protectedClose(_cgi.cgiOUT[1]);
		_epoll.setNonBlocking(_cgi.cgiIN[1]);
		_epoll.setNonBlocking(_cgi.cgiOUT[0]);
		_epoll.addOUTEpoll(_cgi.cgiIN[1]);
		_epoll.addToEpoll(_cgi.cgiOUT[0]);
		_response.cgi = true;
	}
	else
	{
		_response.cgi = false;
		_cgi.closeAllPipes();
		setErrorResponse(eHttpStatusCode::InternalServerError, "failed to fork");
		return ;
	}
}
