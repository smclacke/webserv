/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/15 15:56:08 by smclacke      ########   odam.nl         */
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
				config = std::string(argv[1]);
			Webserv wserv(config);
			for (size_t i = 0; i < wserv.getServerCount(); ++i)
			{
				Server serv = wserv.getServer(i);
				wserv.addServerToEpoll(serv);
			}
			wserv.monitorServers(wserv.getallServer());
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
