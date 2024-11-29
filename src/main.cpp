/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/29 18:50:01 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"
#include "../include/webserv.hpp"
#include "../include/error.hpp"

// Atomic flag to indicate when to stop the program
std::atomic<bool> keepRunning(true);

// Signal handler function
void signalHandler(int signum) {
	std::cout << "\n~~~ Signal received: " << signum << "\n";
    keepRunning = false; // Set the flag to false to exit the loop
}

int main(int argc, char **argv)
{
	std::signal(SIGINT, signalHandler);
	try
	{
		verifyInput(argc, argv);
		std::string config = "";
		if (argc == 2)
			config = std::string(argv[1]);
		Webserv wserv(config, keepRunning);
		wserv.addServersToEpoll();
		wserv.monitorServers();
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
	return (EXIT_SUCCESS);
}
