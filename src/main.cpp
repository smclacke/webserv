/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/23 12:46:46 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

int main(int argc, char **argv)
{

	/* pre built area */
	{
		run();
	}
	/* in case class is funcitonal */
	{
		if (!init(argc, argv))
			return (EXIT_FAILURE);
		if (argc == 1)
			Webserv server;
		else
			Webserv server(std::string(argv[1]));
		server.start();
	}
	return (EXIT_SUCCESS)
}