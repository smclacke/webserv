/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/09 20:57:41 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

/* (true on success) from httpHandler : readFile - true | cgi = true | readFd \ pid */
//struct s_cgi
//{
//	int		pipeIN[2];
//	int		pipeOUT[2];
//	char	*tmp_in_file;
//	char	*tmp_out_file;
//	int		fd;
//	char	*args[3];
//	char	**env;

//};

void	cgiHandler::makeEnvArray()
{
	//_cgi.env; ... make
}

void httpHandler::cgiResponse(void)
{

	// set cgipaths
	// set envvalues
	// setfds

	_cgi.makeEnvArray();
}

