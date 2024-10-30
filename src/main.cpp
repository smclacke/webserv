/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 17:45:21 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

int main(int argc, char **argv)
{
	{
		try
		{
			verifyInput(argc, argv);
			std::string config = "";
			if (argc == 2)
				std::string config = std::string(argv[1]);
			Webserv wserv(config); // will create 1 default server if there is no config
			wserv.run();
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
