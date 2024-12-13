/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:03:33 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/13 12:17:17 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"

static void freeStrings(char *script, char *path)
{
	if (script != NULL)
		free(script);
	if (path != NULL)
		free(path);
}

/**
 * reads from cgiOUT and stores the output in _responseClient.msg
 */
void Epoll::handleCgiRead(s_cgi &cgi)
{
	char buffer[READ_BUFFER_SIZE];
	int bytesRead;
	memset(buffer, 0, sizeof(buffer));
	cgi.state = cgiState::READING;
	bytesRead = read(cgi.cgiOUT[0], buffer, READ_BUFFER_SIZE - 1);
	if (bytesRead < 0)
	{
		std::cerr << "read() cgi failed\n"; /** @todo remove after testing*/
		cgi.state = cgiState::ERROR;
		return;
	}
	else if (bytesRead == 0) // EOF
	{
		cgi.state = cgiState::READY;
		return;
	}
	cgi.complete = true;
	cgi.output.append(buffer, bytesRead);
	if (bytesRead < READ_BUFFER_SIZE - 1)
	{
		cgi.state = cgiState::READY;
		return;
	}
	return;
}

/**
 * @brief
 * if there is no input closes(cgi.cgiIN[1])
 * if there is input writes to cgi.cgiIN[1] + updates cgi.write_offset
 * sets cgi.state to WRITING when needs more writes
 * sets cgi.state to READY when done writing
 * sets cgi.state to ERROR in case of an error
 */
void Epoll::handleCgiWrite(s_cgi &cgi)
{
	if (cgi.input.size() == 0) // no input to write to child, we close pipe
	{
		cgi.state = cgiState::READY;
		return;
	}
	cgi.state = cgiState::WRITING;
	ssize_t leftover;
	ssize_t sendlen = WRITE_BUFFER_SIZE;
	leftover = cgi.input.size() - cgi.write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;
	int bytesWritten = write(cgi.cgiIN[1], cgi.input.c_str() + cgi.write_offset, sendlen);
	if (bytesWritten < 0)
	{
		cgi.state = cgiState::ERROR;
		return;
	}
	else if (bytesWritten == 0)
	{
		cgi.state = cgiState::READY;
		return;
	}
	cgi.write_offset += bytesWritten;
	if (cgi.write_offset >= cgi.input.size())
	{
		cgi.state = cgiState::READY;
	}
}

void httpHandler::cgiResponse()
{
	if (pipe(_cgi.cgiIN) < 0 || pipe(_cgi.cgiOUT) < 0)
	{
		_cgi.closeAllPipes();
		setErrorResponse(eHttpStatusCode::InternalServerError, "failed to open pipes");
		return;
	}
	_cgi.pid = fork();
	if (_cgi.pid == 0) // child: reads from input pipe + writes to output pipe
	{
		char *scriptName = strdup(_cgi.scriptname.c_str());
		if (scriptName == NULL)
		{
			_cgi.closeAllPipes();
			std::exit(EXIT_FAILURE);
		}
		char *scriptPath = strdup(_request.path.c_str());
		if (scriptPath == NULL)
		{
			_cgi.closeAllPipes();
			freeStrings(scriptName, NULL);
			std::exit(EXIT_FAILURE);
		}
		char *argv[] = {scriptPath, nullptr};

		if (dup2(_cgi.cgiIN[0], STDIN_FILENO) < 0 || dup2(_cgi.cgiOUT[1], STDOUT_FILENO) < 0)
		{
			std::cerr << "dup2() cgi failed\n"; /** @todo remove after testing*/
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
		return;
	}
}
