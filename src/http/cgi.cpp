/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/12 17:58:28 by smclacke      ########   odam.nl         */
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
	char	buffer[READ_BUFFER_SIZE];
	int		bytesRead;

	//memset(buffer, 0, sizeof(buffer)); ?
	bytesRead = read(cgi.cgiOUT[0], buffer, READ_BUFFER_SIZE - 1);
	if (bytesRead < 0)
	{
		write(cgi.client_fd, BAD_CGI, BAD_SIZE);
		std::cerr << "read() cgi failed\n"; /** @todo remove after testing*/
		cgi.close = true;
		return ;
	}
	else if (bytesRead == 0) // EOF
	{
		close(cgi.cgiOUT[0]);
		cgi.cgiOUT[0] = -1;
		//cgi.close = true; ?
		cgi.state = cgiState::READY;
		return ;
	}
	cgi.input.append(buffer, bytesRead);
	std::cout << "cgi READ input = " << cgi.input << "\n\n";
	cgi.state = cgiState::READY;
	return ;
	//modifyEvent(cgi.client_fd, EPOLLOUT);
	//cgi.output = true;
}

void	Epoll::handleCgiWrite(s_cgi &cgi)
{
	//std::cout << "write input size = " << cgi.input.size() << "\n\n";
	std::cout << "cgi WRITE input = " << cgi.input << "\n\n";
	if (cgi.input.size() == 0)
	{
		modifyEvent(cgi.client_fd, EPOLLIN);
		cgi.close = false;
		return ;
	}
	int bytesWritten = write(cgi.client_fd, cgi.input.c_str(), cgi.input.size());

	if (bytesWritten < 0)
	{
		std::cerr << "write() to cgi failed\n"; /** @todo remove after testing*/
		cgi.close = true;
		return ;
	}
	//cgi.input.erase(0, bytesWritten);
	if (!cgi.input.empty())
		modifyEvent(cgi.client_fd, EPOLLIN);
	
	else if (cgi.cgiOUT[0] == -1) // everything has been sent
	{
		cgi.close = true;
		return ;
	}
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
