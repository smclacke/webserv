/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/12 11:32:26 by smclacke      ########   odam.nl         */
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
	size_t	readSize = READ_BUFFER_SIZE;
	char	buffer[readSize];
	int		bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));
	cgi.state = cgiState::READING;
	bytesRead = recv(cgi.cgiOUT[0], buffer, readSize - 1, 0);
	if (bytesRead < 0)
	{
		send(cgi.client_fd, BAD_CGI, BAD_SIZE, 0);
		std::cerr << "recv() cgi failed\n"; /** @todo remove after testing*/
		cgi.state = cgiState::ERROR;
		cgi.close = true;
		return ;
	}
	else if (bytesRead == 0)
	{
		if (cgi.pid != -1)
		{
			int status;
			waitpid(cgi.pid, &status, 0);
			cgi.pid = -1;
			if (!cgi.output)
			{
				if (status != 0)
					send(cgi.client_fd, BAD_CGI, BAD_SIZE, 0);
				else
					send(cgi.client_fd, GOOD_CGI, GOOD_SIZE, 0);
			}
		}
		cgi.state = cgiState::CLOSE;
		cgi.close = true;
		return ;
	}
	cgi.output = true;
	buffer[bytesRead - 1] = '\0';
 
	if (bytesRead == READ_BUFFER_SIZE - 1)
	{
		int bytesSend = send(cgi.client_fd, buffer, bytesRead, 0);
		if (bytesSend < 0)
		{
			std::cerr << "write failed\n"; /** @todo remove after testing*/
			cgi.state = cgiState::CLOSE;
			cgi.close = true;
			return ;
		}
	}
	else if (bytesRead < READ_BUFFER_SIZE)
	{
		int bytesSend = send(cgi.cgiOUT[0], buffer, bytesRead, 0);
		if (bytesSend < 0)
		{
			std::cerr << "write failed\n"; /** @todo remove after testing*/
		}
		cgi.state = cgiState::CLOSE;
		cgi.close = true;
	}
}

void	Epoll::handleCgiWrite(s_cgi &cgi)
{
	if (cgi.input.size() == 0)
	{
		cgi.state = cgiState::READING;
		protectedClose(cgi.cgiIN[1]);
		cgi.close = false;
		return ;
	}
	ssize_t leftover;
	ssize_t sendlen = WRITE_BUFFER_SIZE;
	leftover = cgi.input.size() - cgi.write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;
	int bytesWritten = send(cgi.cgiIN[1], cgi.input.c_str() + cgi.write_offset, sendlen, 0);

	if (bytesWritten < 0)
	{
		if (cgi.pid != -1)
		{
			int status;
			waitpid(cgi.pid, &status, 0);
			cgi.pid = -1;
			send(cgi.client_fd, BAD_CGI, BAD_SIZE, 0);
		}
		std::cerr << "send() to cgi failed\n"; /** @todo remove after testing*/
		cgi.state = cgiState::ERROR;
		cgi.close = true;
		return ;
	}
	cgi.write_offset += bytesWritten;
	if (cgi.write_offset >= cgi.input.length())
	{
		cgi.state = cgiState::READING;
		protectedClose(cgi.cgiIN[1]);
		cgi.close = false;
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
