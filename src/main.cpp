/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/23 10:56:21 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"
#include "../include/webserv.hpp"
#include "../include/error.hpp"

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
// 			"GET / HTTP/1.1\r\n"
// 			"Host: example.com\r\n"
// 			"\r\n",

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
// 		for (auto &request : testCases)
// 		{
// 			std::shared_ptr<Server> serv = wserv.getServer(0);
// 			std::stringstream requestStream(request);
// 			std::string response = serv->handleRequest(requestStream); // Pass requestStream directly
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

int main(int argc, char **argv)
{
	try
	{
		verifyInput(argc, argv);
		std::string config = "";
		if (argc == 2)
			config = std::string(argv[1]);
		Webserv wserv(config);
		wserv.addServersToEpoll();
		wserv.monitorServers(wserv.getAllServers());
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
