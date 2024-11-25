/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgiRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/23 00:24:46 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

std::string httpHandler::cgiRequest(void)
{
	std::cout << "It is a CGI request with these inputs:" << std::endl;
	return (writeResponse());
}

// /**
//  * CGI
//  *

// A Web server that supports CGI can be configured to interpret a URL that it serves as a reference
// to a CGI script. A common convention is to have a cgi-bin/ directory at the base of the directory
// tree and treat all executable files within this directory (and no other, for security) as CGI scripts.
// When a Web browser requests a URL that points to a file within the CGI directory
// (e.g., http://example.com/cgi-bin/printenv.pl/with/additional/path?and=a&query=string),
// then, instead of simply sending that file (/usr/local/apache/htdocs/cgi-bin/printenv.pl) to the Web browser,
// the HTTP server runs the specified script and passes the output of the script to the Web browser.
// That is, anything that the script sends to standard output is passed to the Web client instead
// of being shown in the terminal window that started the web server.

// Another popular convention is to use filename extensions; for instance,
// if CGI scripts are consistently given the extension .cgi,
// the Web server can be configured to interpret all such files as CGI scripts.
// While convenient, and required by many prepackaged scripts,
// it opens the server to attack if a remote user can upload executable code with the proper extension.
// The CGI specification defines how additional information passed with the request is passed to the script.
// The Web server creates a subset of the environment variables passed to it and
// adds details pertinent to the HTTP environment.
// For instance, if a slash and additional directory name(s) are appended to the URL immediately
// after the name of the script (in this example, /with/additional/path),
// then that path is stored in the PATH_INFO environment variable before the script is called.
// If parameters are sent to the script via an HTTP GET request (a question mark appended to the URL,
// followed by param=value pairs; in the example, ?and=a&query=string),
// then those parameters are stored in the QUERY_STRING environment variable before the script is called.
// Request HTTP message body, such as form parameters sent via an HTTP POST request,
// are passed to the script's standard input.
// The script can then read these environment variables or data from standard input and
// adapt to the Web browser's request.[8]

// Do you wonder what a CGI is?
// ∗ Because you won’t call the CGI directly, use the full path as PATH_INFO.
// ∗ Just remember that, for chunked request, your server needs to unchunk
// it, the CGI will expect EOF as end of the body.
// ∗ Same things for the output of the CGI. If no content_length is returned
// from the CGI, EOF will mark the end of the returned data.
// ∗ Your program should call the CGI with the file requested as first argument.
// ∗ The CGI should be run in the correct directory for relative path file access.
// ∗ Your server should work with one CGI (php-CGI, Python, and so forth).

//  */