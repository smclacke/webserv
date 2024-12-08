
// old piping, reading, writing of regular file from GET.cpp

/**
 * @brief opens a pipe for the file and sets the outfile descriptor to _response.outFd
 */
void httpHandler::setFile(void)
{
	try
	{
		_response.headers[eResponseHeader::ContentLength] = std::to_string(std::filesystem::file_size(_request.path));
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to retrieve file size: " + std::string(e.what()));
		return;
	}

	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to create pipe");
		return;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to fork process");
		close(pipefd[0]);
		close(pipefd[1]);
		return;
	}
	if (pid == 0)
	{					  // Child process
		close(pipefd[0]); // Close unused read end
		int fileFd = open(_request.path.c_str(), O_RDONLY);
		if (fileFd == -1)
		{
			std::cerr << "Opening file failed" << std::endl;
			setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
			close(pipefd[1]);
			exit(EXIT_FAILURE);
		}
		s_httpSend intro = writeResponse(false);
		std::cout << "\n-------\n" << intro.msg << "\n-------\n" << std::endl;
		
		char buffer[READ_BUFFER_SIZE];
		ssize_t bytesRead;
		size_t totalBytesWritten = 0;
		size_t messageLength = intro.msg.size();
		while (totalBytesWritten < messageLength)
		{
			ssize_t bytesWritten = write(pipefd[1], intro.msg.c_str() + totalBytesWritten, messageLength - totalBytesWritten);
			if (bytesWritten <= 0)
			{
				setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to write to pipe");
				close(fileFd);
				close(pipefd[1]);
				exit(EXIT_FAILURE);
			}
			totalBytesWritten += bytesWritten;
		}
		while ((bytesRead = read(fileFd, buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[READ_BUFFER_SIZE - 1] = '\0';
			std::cout << "READ FROM BUFFER: " << buffer << std::endl;
			if (write(pipefd[1], buffer, bytesRead) == -1)
			{
				//std::cout << "WRITING FAILED" << std::endl;
				setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to write to pipe");
				close(fileFd);
				close(pipefd[1]);
				exit(EXIT_FAILURE);
			}
		}
		close(fileFd);
		close(pipefd[1]);
		exit(EXIT_SUCCESS);
	}
	else
	{
		close(pipefd[1]);					   // Close unused write end
		fcntl(pipefd[0], F_SETFL, O_NONBLOCK); // Set the read end to non-blocking
		_response.readFd = pipefd[0];
		_response.pid = pid;
		_response.readFile = true;
	}
}



// old but working massive handleWrite function that also handles regular files

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
			std::cerr << "Write to client failed\n";
			client._connectionClose = true;
			return;
		}
		else if (bytesWritten == 0)
		{
			//std::cout << "Client disconnected\n";
			client._connectionClose = true;
			return;
		}
		client._write_offset += bytesWritten;
		if (client._write_offset >= client._responseClient.msg.length())
		{
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
		// to handle file, need to add to epoll right?? this doesnt work... :)
		//addToEpoll(client._responseClient.readFd);
		
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
			if (client._responseClient.pid != -1)
			{
				int status;
				waitpid(client._responseClient.pid, &status, 0);
			}
			client._responseClient.readFd = -1;
			client._responseClient.pid = -1;
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
			if (client._responseClient.pid != -1)
			{
				int status;
				waitpid(client._responseClient.pid, &status, 0);
			}
			client._responseClient.readFd = -1;
			client._responseClient.pid = -1;
			if (client._responseClient.keepAlive == false)
				client._connectionClose = true;
			return;
		}
	}
}


// old add file to epll that obvs doesnt work :)

void		Epoll::addToEpollFile(int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET; // Edge-triggered
	event.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		perror("epoll_ctl");
		std::cerr << "epoll_ctl error: add fd: " << fd << std::endl;
		protectedClose(fd);
	}
	return ;
}


// old handle read

void	Epoll::handleRead(t_clients &client)
{
	char buffer[READ_BUFFER_SIZE];
	size_t bytesRead = 0;
	memset(buffer, 0, sizeof(buffer));

	client._clientState = clientState::READING;
	bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
	
	// Error
	if (bytesRead < 0)
	{
		std::cerr << "Reading from client connection failed\n";
		client._clientState = clientState::ERROR;
		client._connectionClose = true;
		return ;
	}
	
	// Disconnected
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected\n";
		client._clientState = clientState::CLOSE;
		client._connectionClose = true;
		return ;
	}

	buffer[READ_BUFFER_SIZE - 1] = '\0';
	std::string buf = buffer;
	client._requestClient.append(buf);

	// FInished
	if (client._requestClient.find("\r\n\r\n") != std::string::npos)
	{
		client._clientState = clientState::READY;
		client._connectionClose = false;
		return ;
	}
	client._connectionClose = false;
}
