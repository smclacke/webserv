/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/11 15:32:34 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

void		Epoll::closeAllPipes(int cgiIN[2], int cgiOUT[2])
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

/**  @todo check - cgi has nothing to do with chunking? so only need read buffer size? */
void	Epoll::handleCgiRead(httpHandler &cgi_http, int OutPipe)
{
	size_t	readSize = READ_BUFFER_SIZE;
	char	buffer[readSize];
	int		bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));

	cgi.state = cgiState::READING;
	bytesRead = recv(outPipe, buffer, readSize - 1, 0);

	if (bytesRead < 0)
	{
		std::cerr << "recv() cgi failed\n";
		cgi.state = cgiState::ERROR;
		cgi.close = true;
		return ;
	}
	else if (bytesRead == 0)
	{
		cgi.state = cgiState::CLOSE;
		cgi.close = true;
		return ;
	}
	
	buffer[readSize - 1] = '\0';
	std::string buf = buffer;

	/* alternative: ?*/

	/** @todo check */
	// really not sure about this
	cgi_http.addStringBuffer(buf);
	if (cgi_http.getKeepReading())
		return ;
	else
	{
		cgi.state = cgiState::READY;
		cgi.close = false;
	}
}

void	Epoll::handleCgiWrite(httpHandler &cgi_http, int InPipe)
{
	if (cgi.response.msg.empty())
	{
		cgi.response = cgi_http.generateResponse();
		cgi.state = cgiState::WRITING;
		cgi_http.clearHandler();
	}
	ssize_t leftover;
	ssize_t sendlen = WRITE_BUFFER_SIZE;
	leftover = cgi.response.msg.size() - cgi.write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;

	int bytesWritten = send(inPipe, cgi.response.msg.c_str() + cgi.write_offset, sendlen, 0);

	if (bytesWritten < 0)
	{
		std::cerr << "send() to cgi failed\n";
		cgi.state = cgiState::ERROR;
		cgi.close = true;
		return ;
	}
	else if (bytesWritten == 0)
	{
		cgi.state = cgiState::CLOSE;
		cgi.close = true;
		return ;
	}

	cgi.write_offset += bytesWritten;

	// Finished
	if (cgi.write_offset >= cgi.response.msg.length())
	{
		cgi.state = cgiState::READY;
		if (cgi.response.keepAlive == false)
		{
			cgi.state = cgiState::CLOSE;
			cgi.close = true;
		}
		if (cgi.response.keepAlive)
			cgi.close = false;
		cgi.write_offset = 0;
		cgi.response.msg.clear();
		return ;
	}
	cgi.close = false;
}

void		httpHandler::cgiResponse()
{
	if (pipe(_cgi.cgiIN) < 0 || pipe(_cgi.cgiOUT) < 0)
	{
		_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
		std::cerr << "pip() cgi failed\n";
		return ;
	}
	_response.pid = fork();
	if (_response.pid == 0) // child
	{
		char	*scriptName = strdup(_cgi.scriptname.c_str());
		if (scriptName == NULL)
		{
			_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
			throw std::runtime_error("failed malloc");
		}
		char	*scriptPath = strdup(_request.path.c_str());
		if (scriptPath == NULL)
		{
			_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
			freeStrings(scriptName, NULL);
			throw std::runtime_error("failed malloc");
		}

		char	*argv[] = {scriptPath, nullptr};
		
		 // child reads from input pipe + writes to output pipe
		if (dup2(_cgi.cgiIN[0], STDIN_FILENO) < 0 || dup2(_cgi.cgiOUT[1], STDOUT_FILENO) < 0)
		{
			std::cerr << "dup2() cgi failed\n";
			_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
			freeStrings(scriptName, scriptPath);
			exit(EXIT_FAILURE);
		}

		// close unused fds
		protectedClose(_cgi.cgiIN[1]);
		protectedClose(_cgi.cgiOUT[0]);

		// execute cgi script
		if (execve(scriptPath, argv, _cgi.env.data()) == -1)
		{
			std::cerr << "execve failed\n";
			freeStrings(scriptName, scriptPath);
			_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
			exit(EXIT_FAILURE);	
		}

		_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
		freeStrings(scriptName, scriptPath);
		exit(EXIT_SUCCESS);
	}
	else if (_response.pid > 0) // parent
	{
		/* parent only writes to input and reads from output*/
		// close unused fds
		protectedClose(_cgi.cgiIN[0]);
		protectedClose(_cgi.cgiOUT[1]);
	
		// add to monitor
		_epoll.addOUTEpoll(_cgi.cgiIN[1]);
		_epoll.addToEpoll(_cgi.cgiOUT[0]);
		
	}
	else // error
	{
		_epoll.closeAllPipes(_cgi.cgiIN, _cgi.cgiOUT);
		std::cerr << "fork() cgi failed\n";
		return ;
	}
}
