/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgiHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/09 20:49:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 14:41:34 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"

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


class	cgiHandler
{
	private:
		int					pipeIN[2];
		int					pipeOUT[2];
		std::string			file;
		//std::string			py_file; // python file from usr/bin/python
		//std::string			php_file; // php file from usr/bin/php
		char				*tmp_in_file;
		char				*tmp_out_file;
		std::string			cur_dir;
		std::string			cgi_path;
		int					fd;
		char				*args[3];
		bool				yesPy;
		bool				yesPhp;

	public:
		cgiHandler::cgiHandler();
		cgiHandler::~cgiHandler();


	/* Methods */
	char				**makeEnvArray();

	/* setters*/
	void				setPaths(s_request &_request);
	void				setEnvValues();
	void				setFds();

	/* getters*/
	std::string			getCurDir();
		
};

#endif /* CGI_HANDLER_HPP */
