/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/29 17:50:33 by jde-baai      ########   odam.nl         */
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
		try
		{
			verifyInput(argc, argv);
			std::string config = "";
			if (argc == 2)
				std::string config = std::string(argv[1]);
			Webserv wserv(config);
			Server first = wserv.getServer(0);
			first.printServer();
		}
		catch (eConf &e)
		{
			std::cout << "Config-Error: line: " << e.line() << " error: " << e.what() << std::endl;
		}
		catch (std::runtime_error &e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
		catch (std::logic_error &e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}
	return (EXIT_SUCCESS);
}