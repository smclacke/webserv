/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/13 16:12:11 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"
#include "../include/webserv.hpp"
#include "../include/server.hpp"
#include "../include/error.hpp"

// Atomic flag to indicate when to stop the program
std::atomic<bool> keepRunning(true);

/**
 * @brief ends the program when ^C is received
 */
void signalHandler(int signum)
{
	(void)signum;
	std::cout << "\nprogram terminated by signal\n";
	keepRunning = false;
}

int main(int argc, char **argv)
{
	std::signal(SIGINT, signalHandler);
	try
	{
		if (argc > 2)
			throw std::runtime_error("too many inputs, provide 1 *.conf file");
		std::string config = "./config_files/default.conf";
		if (argc == 2)
			config = std::string(argv[1]);
		verifyInput(config);
		Webserv wserv(config, keepRunning);
		wserv.monitorServers();
	}
	catch (eConf &e)
	{
		if (e.line() != -1)
			std::cerr << "Config-Error: line: " << e.line() << " error: " << e.what() << std::endl;
		else
			std::cerr << "Config-Error: " << e.what() << std::endl;
	}
	catch (std::runtime_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (std::logic_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (std::bad_alloc &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return (EXIT_SUCCESS);
}
