/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 16:03:08 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

///* constructors an' that */
//cgiHandler::cgiHandler() //: yesPy(false), yesPhp(false)
//{}

//cgiHandler::~cgiHandler()
//{
//	// clean up all files + pipes + args + fds (+ envs?)
//}

/* methods */
void httpHandler::cgiResponse(std::vector<char *> env)
{
	(void) env;
	
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
	if (_response.pid < 0)
	{
		closeAllPipes(cgiIN, cgiOUT);
		protectedClose(cgiIN[0]);
		protectedClose(cgiIN[1]);
		protectedClose(cgiOUT[0]);
		protectedClose(cgiOUT[1]);
		
	}
	
	//_cgi.setPaths(_request);
	//_cgi.setEnvValues(_request); // need request?
	//_cgi.setFds(); // need request?
	//char **env = _cgi.makeEnvArray();

	

	// clean up env
}

/* setters */
//void	cgiHandler::setPaths()
//{
//	// get location struct, match the location

//	//file = _request. // where do i get the requested file from?
//	//std::cout << "cgi file = " << file << "\n";

//	// gettempfilepath - also from http? either /usr/bin/python or php
//	// cgi_path = 
//	//std::cout << "cgi path = " << cgi_path << "\n";

//	// get current directory
//	//cur_dir = getCurDir();
//	//std::cout << "cur dir = " << cur_dir << "\n";
	
//	//if (cgi_path == "/usr/bin/python")
//	//{
//	//	cgi_path = file;
//		yesPy = true;
//	//	return ;
//	//}
//	//else if (cgi_path == "/usr/bin/php")
//	//{
//	//	cgi_path = file;
//		yesPhp = true;
//	//	return ;
//	//}
//	//else
//	//	std::cerr << "incorrect path\n";
	
//}


//void	cgiHandler::setFds()
//{
	
//}

///* getters */
//std::string		cgiHandler::getCurDir()
//{
//	/** @todo get current directory */

//	//return cur_dir;
//}
