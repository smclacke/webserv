/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/06 11:27:26 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"
#include "../include/webserv.hpp"
#include "../include/server.hpp"
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

// int main(int argc, char **argv)
// {
// 	try
// 	{
// 		verifyInput(argc, argv);
// 		std::string config = (argc == 2) ? std::string(argv[1]) : "";
// 		Webserv wserv(config);

// 		// Test cases
// 		std::vector<std::string> testCases = {
// 			// Basic GET Request
// 			"GET /loc2dir/new_route/content.html HTTP/1.1\r\n"
// 			"Host: example.com\r\n"
// 			"Accept: text/html"
// 			"\r\n",

// 			"POST /loc2dir HTTP/1.1\r\n"
// 			"Host: example.com\r\n"
// 			"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary\r\n"
// 			"Content-Length: 138\r\n"
// 			"\r\n"
// 			"------WebKitFormBoundary\r\n"
// 			"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
// 			"Content-Type: text/plain\r\n"
// 			"\r\n"
// 			"This is a test file.\r\n"
// 			"------WebKitFormBoundary--\r\n"

// 			// Basic POST Request
// 			// "POST /submit-form HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "Content-Type: application/x-www-form-urlencoded\r\n"
// 			// "Content-Length: 17\r\n"
// 			// "\r\n"
// 			// "name=John&age=30",

// 			// // DELETE Request
// 			// "DELETE /resource/123 HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "\r\n",

// 			// // Chunked Transfer Encoding
// 			// "POST /upload HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "Transfer-Encoding: chunked\r\n"
// 			// "\r\n"
// 			// "4\r\n"
// 			// "Wiki\r\n"
// 			// "5\r\n"
// 			// "pedia\r\n"
// 			// "0\r\n"
// 			// "\r\n",

// 			// // Redirection
// 			// "GET /old-route HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "\r\n",

// 			// // Directory Listing
// 			// "GET /files/ HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "\r\n",

// 			// // CGI Execution
// 			// "GET /cgi-bin/script.php HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "\r\n",

// 			// // File Upload
// 			// "POST /upload HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary\r\n"
// 			// "Content-Length: 138\r\n"
// 			// "\r\n"
// 			// "------WebKitFormBoundary\r\n"
// 			// "Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
// 			// "Content-Type: text/plain\r\n"
// 			// "\r\n"
// 			// "This is a test file.\r\n"
// 			// "------WebKitFormBoundary--\r\n",

// 			// // Error Handling
// 			// "GET /nonexistent HTTP/1.1\r\n"
// 			// "Host: example.com\r\n"
// 			// "\r\n"
// 		};
// 		// Process each test case
// 		std::shared_ptr<Server> serv = wserv.getServer(0);
// 		serv->printServer();
// 		for (auto &request : testCases)
// 		{
// 			std::stringstream requestStream(request);
// 			s_httpSend http = serv->handleRequest(requestStream);
// 			std::string response = http.msg; // Pass requestStream directly
// 			std::cout << "\nRequest:\n"
// 					  << request << "\nResponse:\n"
// 					  << response << std::endl;
// 		}
// 	}
// 	catch (eConf &e)
// 	{
// 		std::cout << "Config-Error: line: " << e.line() << " error: " << e.what() << std::endl;
// 	}
// 	catch (std::runtime_error &e)
// 	{
// 		std::cout << "Error: " << e.what() << std::endl;
// 	}
// 	catch (std::logic_error &e)
// 	{
// 		std::cout << "Error: " << e.what() << std::endl;
// 	}
// 	return (EXIT_SUCCESS);
// }
