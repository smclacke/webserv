

// old writing function

void	Epoll::handleWrite( t_clients &client)
{
	// Get the request response if we are here for the first time
	client._clientState = clientState::WRITING;
	ssize_t		leftover;
	ssize_t		sendlen = WRITE_BUFFER_SIZE;
	leftover = client._responseClient.msg.size() - client._write_offset;
	if (leftover < WRITE_BUFFER_SIZE)
		sendlen = leftover;

	ssize_t bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, leftover, 0);
	
	// Error
	if (bytesWritten < 0)
	{
		std::cerr << "Write to client failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		return ;
	}

	// Disconnected
	else if (bytesWritten == 0)
	{
		//std::cout << "Client disconnected\n";
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return ;
	}

	client._write_offset += bytesWritten;
	
	// Finished
	if (client._write_offset >= client._responseClient.msg.length())
	{
		if (client._responseClient.keepAlive == false)
		{
			client._clientState = clientState::CLOSE;
			client._connectionClose = true;
		}
		client._clientState = clientState::READY;
		client._connectionClose = false;
		client._write_offset = 0;
		client._responseClient.msg.clear();
		return ;
	}
	client._connectionClose = false;
}

// old file function

void	Epoll::handleFile(t_clients &client)
{
	client._clientState = clientState::WRITING;
	ssize_t		bytesSend;
	char		buffer[READ_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	// i guess im not reading here anymore?
	
	ssize_t		bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE - 1);
	// Error
	if (bytesRead < 0)
	{
		std::cerr << "Reading from pipe failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		return ;
	}

	// Nothing to read -> we are done
	else if (bytesRead == 0)
	{
		client._readingFile = false;
		client._clientState = clientState::READY;
		if (client._responseClient.pid != -1)
		{
			int status;
			waitpid(client._responseClient.pid, &status, 0);
		}
		client._responseClient.pid = -1;
		if (client._responseClient.keepAlive == false)
		{
			client._clientState = clientState::CLOSE;
			client._connectionClose = true;
		}
		return ;
	}
	buffer[READ_BUFFER_SIZE - 1] = '\0';
	std::cout << "Read from pipe: " << buffer << std::endl;
	if (bytesRead == READ_BUFFER_SIZE - 1) // we are not done
	{
		bytesSend = send(client._fd, buffer, bytesRead, 0);
		if (bytesSend < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return;
		}
		std::cout << "BYTES SEND: " << bytesSend << std::endl;
	}
	else if (bytesRead < READ_BUFFER_SIZE) // we need to send and then we are done
	{
		bytesSend = send(client._fd, buffer, bytesRead, 0);
		if (bytesSend < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return ;
		}
		client._readingFile = false;
		client._clientState = clientState::READY;

		if (client._responseClient.pid != -1)
		{
			int status;
			waitpid(client._responseClient.pid, &status, 0);
		}
		client._responseClient.pid = -1;
		if (client._responseClient.keepAlive == false)
			client._connectionClose = true;
		return ;
	}
}


// old big write

/** @todo remove when handleWrite and handleFile all good */
void	Epoll::handleBigWrite(t_serverData &server, t_clients &client)
{
	if (client._responseClient.msg.empty() && client._readingFile == false)
	{
		client._responseClient = server._server->handleRequest(client._requestClient);
		client._clientState = clientState::WRITING;
		client._requestClient.clear();
	}
	
	// not handling file
	if (client._readingFile == false)
	{
		ssize_t leftover;
		ssize_t sendlen = WRITE_BUFFER_SIZE;
		leftover = client._responseClient.msg.size() - client._write_offset;
		if (leftover < WRITE_BUFFER_SIZE)
		{
			sendlen = leftover;
		}
		ssize_t bytesWritten = send(client._fd, client._responseClient.msg.c_str() + client._write_offset, leftover, 0);
		if (bytesWritten < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return;
		}
		else if (bytesWritten == 0)
		{
			std::cout << "Client disconnected\n";
			client._connectionClose = true;
			return;
		}
		client._write_offset += bytesWritten;
		if (client._write_offset >= client._responseClient.msg.length())
		{
			std::cout << "Finished sending message, hora!\n";
			if (client._responseClient.readFd != -1)
			{
				client._readingFile = true;
				client._connectionClose = false;
				return ;
			}
			else
			{
				client._clientState = clientState::READY;
				if (client._responseClient.keepAlive == false)
					client._connectionClose = true;
			}
			client._write_offset = 0;
			client._responseClient.msg.clear();
			return ;
		}
		client._connectionClose = false;
	}
	else // handling file
	{

		ssize_t bytesSend;
		char buffer[READ_BUFFER_SIZE];
		ssize_t bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE - 1);
		if (bytesRead < 0) // error
		{
			std::cerr << "Reading from pipe failed\n";
			client._connectionClose = true;
			return;
		}
		else if (bytesRead == 0) // nothing to read anymore -> we are done
		{
			client._readingFile = false;
			client._clientState = clientState::READY;
			close(client._responseClient.readFd);
			client._responseClient.readFd = -1;
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;
			return;
		}
		buffer[READ_BUFFER_SIZE - 1] = '\0';
		if (bytesRead == READ_BUFFER_SIZE - 1) // we are not done
		{
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return;
			}
		}
		else if (bytesRead < READ_BUFFER_SIZE) // we need to send and then we are done
		{
			bytesSend = send(client._fd, buffer, bytesRead, 0);
			if (bytesSend < 0)
			{
				std::cerr << "Write to client failed\n";
				client._connectionClose = true;
				return;
			}
			client._readingFile = false;
			client._clientState = clientState::READY;
			close(client._responseClient.readFd);
			client._responseClient.readFd = -1;
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;
			return;
		}
	}
}



/**
 * @brief Builds the response based on the information in s_response;
 */
s_httpSend httpHandler::writeResponse(bool keepalive)
{
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
	{
		std::cout << "BODY = " << _response.body.str() << std::endl;
		std::stringstream responseStream;
		// status line
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " "
					   << statusMessages.at(_statusCode) << "\r\n";
		if (_response.readFile)
		{
			_response.headers[eResponseHeader::ContentLength] = std::to_string(std::filesystem::file_size(_response.filepath));
		}
		else if (!_response.body.str().empty())
		{
			if (_response.headers.find(eResponseHeader::ContentLength) == _response.headers.end())
				_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
		}
		// headers
		if (keepalive)
			_response.headers[eResponseHeader::Connection] = "keep-alive";
		else
			_response.headers[eResponseHeader::Connection] = "close";
		for (const auto &header : _response.headers)
		{
			responseStream << responseHeaderToString(header.first) << header.second << "\r\n";
		}
		// End headers section
		responseStream << "\r\n";
		// body
		if (_response.readFile == false && _response.cgi == false)
		{
			responseStream << _response.body.str();
		}
		s_httpSend response = {responseStream.str(), keepalive, _response.filepath, _response.readFile, _response.readFd, _response.pid};
				//s_httpSend response = {responseStream.str(), _response.keepalive, _response.readFile, _response.readFd, _response.cgi, _response.pid};
		//s_httpSend response = {responseStream.str(), _response.keepalive, _response.readFile, _response.readFd, _response.cgi, _response.pid};
		//s_httpSend response = {responseStream.str(), keepalive, _response.filepath, _response.readFile, _response.readFd, _response.pid};
		return (response);
	}
	else
	{
		_statusCode = eHttpStatusCode::BadRequest;
		std::string message = "Bad request";
		std::ostringstream responseStream;
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " " << message << "\r\n"
					   << "Content-Type: text/plain\r\n"
					   << "Content-Length: " << message.size() << "\r\n"
					   << "Connection: close\r\n"
					   << "\r\n"
					   << message;
		s_httpSend response = {responseStream.str(), keepalive, "", false, -1, -1};
		return (response);
	}
}


// old GET.cpp readFile

/**
 * @brief opens a pipe for the file and sets the outfile descriptor to _response.outFd
 */
void httpHandler::readFile(void)
{
	//int pipefd[2];
	//if (pipe(pipefd) == -1)
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to create pipe");
	//	return;
	//}

	//pid_t pid = fork();
	//if (pid == -1)
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to fork process");
	//	close(pipefd[0]);
	//	close(pipefd[1]);
	//	return;
	//}
	//if (pid == 0)
	//{					  // Child process
	//	close(pipefd[0]); // Close unused read end
	//	int fileFd = open(_request.path.c_str(), O_RDONLY);
	//	if (fileFd == -1)
	//	{
	//		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
	//		close(pipefd[1]);
	//		exit(EXIT_FAILURE);
	//	}
	//	exit(EXIT_FAILURE);			   // If execlp fails
	//}
	//else
	//{								  // Parent process
	//	close(pipefd[1]);			  // Close unused write end
	//	_response.readFd = pipefd[0]; // Set the read end of the pipe for epoll
	//	_response.pid = pid;
	//	_response.readFile = true;
	//}
	int out = open(_request.path.c_str(), O_RDONLY);
	if (out == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
		return;
	}
	//std::cout << "path =====" << _request.path << std::endl;
	//std::ifstream is(_request.path);
	//if (!is.is_open())
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
	//	return;	
	//}
	//std::string line;
	//std::getline(is, line); // if end of file returns false
	//if (is.fail())
	//{
	//	perror("bruh:");
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "getline error");
	//	return;	
	//}
	//std::cout << "Read with getline =" << line << std::endl;
	//std::cout << "readfd in GET:" << out << std::endl;
	//_response.readFd = out;
	_response.filepath = _request.path;
	_response.readFd = out;
	_response.readFile = true;
}
