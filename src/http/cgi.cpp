/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 14:56:50 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

/* constructors an' that */
cgiHandler::cgiHandler() : yesPy(false), yesPhp(false)
{
	// clear all files + pipes
	//+ args + fds + envs
}

cgiHandler::~cgiHandler()
{
	// clean up all files + pipes + args + fds + envs
}

/* methods */

char	**cgiHandler::makeEnvArray()
{
	char						**envp;
	std::vector<std::string>	env_v;

	// need request stuff
	// fill envp with env ...
	
}

void httpHandler::cgiResponse(void)
{
	// need the request from the httphandler + input body
	// request headers

	_cgi.setPaths(_request);
	//_cgi.setEnvValues(_request); // need request?
	//_cgi.setFds(); // need request?
	char **env = _cgi.makeEnvArray();

	

	// clean up env
}

/* setters */
void	cgiHandler::setPaths(s_request &_request)
{
	// get location struct, match the location

	//file = _request. // where do i get the requested file from?
	//std::cout << "cgi file = " << file << "\n";

	// gettempfilepath - also from http? either /usr/bin/python or php
	// cgi_path = 
	//std::cout << "cgi path = " << cgi_path << "\n";

	// get current directory
	cur_dir = getCurDir();
	std::cout << "cur dir = " << cur_dir << "\n";
	
	if (cgi_path == "/usr/bin/python")
	{
		cgi_path = file;
		yesPy = true;
		return ;
	}
	else if (cgi_path == "/usr/bin/php")
	{
		cgi_path = file;
		yesPhp = true;
		return ;
	}
	else
		std::cerr << "incorrect path\n";
	
}

void	cgiHandler::setEnvValues()
{
	// env.set .. || get ...
}

void	cgiHandler::setFds()
{
	
}

/* getters */
std::string		cgiHandler::getCurDir()
{
	/** @todo get current directory */

	return cur_dir;
}

