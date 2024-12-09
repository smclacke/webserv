

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



