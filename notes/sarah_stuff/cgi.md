
**CGIREAD**


void Epoll::handleCgiRead(s_cgi &cgi)
{
    char buffer[READ_BUFFER_SIZE];
    int bytesRead = read(cgi.cgiOUT[0], buffer, sizeof(buffer));

    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; // No data yet, try later

        std::cerr << "read() failed\n";
        cgi.state = cgiState::ERROR;
        cgi.close = true;
        return;
    }

    if (bytesRead == 0) // EOF
    {
        close(cgi.cgiOUT[0]);
        cgi.cgiOUT[0] = -1;
        cgi.state = cgiState::WRITING;
        return;
    }

    // Append to output buffer
    cgi.outputBuffer.append(buffer, bytesRead);

    // Notify epoll that client_fd is ready for writing
    modifyEpoll(client_fd, EPOLLOUT);
    cgi.state = cgiState::WRITING;
}

void Epoll::handleCgiWrite(s_cgi &cgi)
{
    if (cgi.outputBuffer.empty())
    {
        cgi.state = cgiState::READING;
        modifyEpoll(cgi.client_fd, EPOLLIN);
        return;
    }

    int bytesSent = write(cgi.client_fd, cgi.outputBuffer.c_str(), cgi.outputBuffer.size());

    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; // Client not ready to receive

        std::cerr << "write() failed\n";
        cgi.state = cgiState::ERROR;
        cgi.close = true;
        return;
    }

    // Remove sent data from buffer
    cgi.outputBuffer.erase(0, bytesSent);

    // If buffer is empty, check if we are done
    if (cgi.outputBuffer.empty() && cgi.cgiOUT[0] == -1)
    {
        cgi.state = cgiState::CLOSE;
        cgi.close = true;
    }
}





void Epoll::handleCgiRead(s_cgi &cgi)
{
    char buffer[READ_BUFFER_SIZE];
    int bytesRead = read(cgi.cgiOUT[0], buffer, sizeof(buffer));

    if (bytesRead < 0)
    {
        std::cerr << "read() cgi failed\n";
        write(cgi.client_fd, BAD_CGI, BAD_SIZE);
        cgi.state = cgiState::ERROR;
        cgi.close = true;
        return;
    }

    if (bytesRead == 0) // EOF
    {
        if (cgi.pid != -1)
        {
            int status;
            waitpid(cgi.pid, &status, 0);
            cgi.pid = -1;
            if (!cgi.output && status != 0)
                write(cgi.client_fd, BAD_CGI, BAD_SIZE);
        }
        cgi.state = cgiState::CLOSE;
        cgi.close = true;
        return;
    }

    // Store data for later sending
    cgi.outputBuffer.append(buffer, bytesRead);
    cgi.output = true;

    // Change state and wait for client socket to become writable
    cgi.state = cgiState::WRITING;
}


void Epoll::handleCgiWrite(s_cgi &cgi)
{
    if (cgi.outputBuffer.empty())
    {
        cgi.state = cgiState::CLOSE;
        cgi.close = true;
        return;
    }

    int bytesSent = write(cgi.client_fd, cgi.outputBuffer.c_str(), cgi.outputBuffer.size());

    if (bytesSent < 0)
    {
        std::cerr << "write() failed\n";
        cgi.state = cgiState::ERROR;
        cgi.close = true;
        return;
    }

    // Remove sent data from buffer
    cgi.outputBuffer.erase(0, bytesSent);

    // If all data has been sent, mark for closure
    if (cgi.outputBuffer.empty())
    {
        cgi.state = cgiState::CLOSE;
        cgi.close = true;
    }
}





/**
 * check if file is executable
 * pipe
 * fork
 * in child:
 * 	-close read end
 * 	-Set paramaters
 * 	-execve
 * else
 * 	-close write end
 * 	- set CGI as true
 * 	- set ReadFD
 * 	- set pid
 */

/*
	- args[0] has path of cgi program or script e.g. "/usr/local/bin/php-cgi"
	- args[1] hsa path of cgi file, if script file == NULL
	- args[2] == NULL
	- env has parsed request and variables accoring to RFC(3875) - num pos spec cgi

*/


//void httpHandler::cgiResponse(void)
//{
//	std::cout << "It is a CGI request" << std::endl;
	
//	s_cgi	cgi;
//	cgi.args[0] = NULL; // path of cgi program
//	cgi.args[1] = NULL;
//	cgi.args[2] = NULL;
	

//	if (pipe(cgi.pipeIN) == -1)
//	{
//		std::cerr << "Pipe() failed for cgi process\n";
//		protectedClose(cgi.fd);
//		return ;
//	}
//	if (pipe(cgi.pipeOUT) == -1)
//	{
//		std::cerr << "Pipe() failed for cgi process\n";
//		protectedClose(cgi.fd);
//		return ;
//	}
//	if ((_response.pid = fork()) == 0) // child
//	{
//		if (dup2(STDIN_FILENO, cgi.pipeIN[0]) == -1)
//		{
//			std::cerr << "dup2() STDIN failed\n";
//			return ;
//		}
//		if (dup2(STDOUT_FILENO, cgi.pipeOUT[1]) == -1)
//		{
//			std::cerr << "dup2() STDOUT failed\n";
//			return ;
//		}
//	}
//	else
//	{
//		_response.cgi = true;
//		_response.readFile = true;
		
//		// request content -> cgi.pipeIN[1];
//		// response content -> cgi.pipeOUT[0];
//		// read & send (epoll?) -> close
		
//	}
//	return ;
//}


/* (true on success) from httpHandler : readFile - true | cgi = true | readFd \ pid */
struct s_cgi
{
	int		pipeIN[2];
	int		pipeOUT[2];
	char	*tmp_in_file;
	char	*tmp_out_file;
	int		fd;
	char	*args[3];
	char	**env;

};

/* using abi as a shining example <3 */

/*
	set cgipaths: (julius might have done some or most of this)

	location (matching location)
	requested file path
	cgi path -> if not e.g. /usr/bin/python
	not valid path...
	
 */

/*
	set envvalues: (julius might have done some or most of this)
	header shit... (dont need right?)
	+ query str + script names + server info etc etc
	request_uri - maybe need
	
 */

/*
	set fds:

	in_fd = dup STDIN
	out_fd = dup STOUT
	tmp_in_file.createTmpFile()
	tmp_out_file.createTmpFile()
	// in_file_fd = fileno(tmp_in_file) // what?
	// out_file_fd = fileno(tmp_out_file) // what?
*/

/*
	making env array:

	char		**envp

	envp= new char*

	yes, i need this...

	CGI Standard: CGI scripts rely on environment variables to 
	receive request-specific information from the web server.
	Process Communication: When executing a CGI script, the server 
	passes these variables through the process environment

 */

/*
	execute script:
	
	dup2 in stdin
	dup2 out stdou
	execve cgi path, null, envp
	
*/

/*
	readscript output: -> let epoll do this

	char buffer[65000] 
	
	waitpid
	// lseek ?? out file fd
	
	while read out fd
		body + buffer
	
 */

/*
	char **envp
	_response.pid

	// set cgipaths
	// set envvalues
	// set fds 

	envp = makeenvarray()

	// reading in the input body from temp file
	write(in_file_fd, input_body, body_length)

	// reset to beginning of file?
	lseek()

	// need to also check the method somewhere?

	// forking for executing script
	pid = fork()
	// == -1 responseheader 500
	pid == 0
		executeScript
	else readscript output -> give fd to epoll

	closee + remove envp

	return hedaer outputbody

 */



