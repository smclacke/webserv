/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/19 14:28:43 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"
#include "../include/webserv.hpp"
#include "../include/error.hpp"

int main(int argc, char **argv)
{
	{
		try
		{
			// default cgi request to unpack
			std::string cgi = "POST /loc2dir/cgi_path/test_file.php HTTP/1.1\r\n"
							  "Host: www.example.com\r\n"
							  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
							  "Content-Type: application/x-www-form-urlencoded\r\n"
							  "Content-Length: 27\r\n"
							  "\r\n"
							  "name=John&age=30\r\n";

			// default POST request to unpack
			std::string std = "POST /images HTTP/1.1\r\n"
							  "Host: www.example.com\r\n"
							  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
							  "Content-Type: application/x-www-form-urlencoded\r\n"
							  "Content-Length: 27\r\n"
							  "\r\n"
							  "name=John&age=30\r\n";
			verifyInput(argc, argv);
			std::string config = "";
			if (argc == 2)
				config = std::string(argv[1]);
			Webserv wserv(config);
			// monitor
			for (size_t i = 0; i < wserv.getServerCount(); ++i)
			{
				std::shared_ptr<Server> serv = wserv.getServer(i);
				serv->printServer();
			}
			std::cout << "\n============================="
					  << std::endl;
			std::shared_ptr<Server> serv = wserv.getServer(0);
			std::string response = serv->handleRequest(std);
			std::cout << "\nResponse:\n"
					  << response << std::endl;
			std::cout << "=============================\n"
					  << std::endl;
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
