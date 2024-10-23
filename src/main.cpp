/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/23 16:45:40 by jde-baai      ########   odam.nl         */
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
		e_config config = verifyInput(argc, argv);
		if (config == BAD_INPUT)
			return (EXIT_FAILURE);
		try
		{
			if (config == DEFAULT)
				initDefault();
			if (config == SERVER_CONF)
				initConf((std::string(argv[1])));
		}
		catch (std::exception e)
		{
		}
	}
	return (EXIT_SUCCESS);
}