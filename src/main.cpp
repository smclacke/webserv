/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/22 15:27:28 by jde-baai      ########   odam.nl         */
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
		if (argc != 2 && !validConf(argv[1]))
			exit(EXIT_FAILURE);
		try
		{
			Webserv server(argv[1]);
		}
		catch (std::exception &e)
		{
		}
	}

	return (EXIT_SUCCESS);
}
