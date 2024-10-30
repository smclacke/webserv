/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/30 15:06:39 by jde-baai      ########   odam.nl         */
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
			size_t server_count = 3;
			for (size_t i = 0; i < server_count; ++i)
			{
				Server nServer;
				wserv.addServer(nServer);
			}
			std::cout << "Created: " << wserv.getServerCount() << " servers" << std::endl;
			Server getServer = wserv.getServer(0);
			getServer.printServer();
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