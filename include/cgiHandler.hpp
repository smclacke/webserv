/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgiHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/09 20:49:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/09 20:57:33 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"

class	cgiHandler
{
	private:
		int		pipeIN[2];
		int		pipeOUT[2];
		char	*tmp_in_file;
		char	*tmp_out_file;
		int		fd;
		char	*args[3];
		char	**env;


	public:


	/* Methods */
	void				makeEnvArray();
		
};

#endif /* CGI_HANDLER_HPP */
