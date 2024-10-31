/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsLocation.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/31 15:42:05 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/31 16:49:09 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

// static void parseMethod(std::string &line, int &line_n, s_location &loc)
// {
// 	/*
// 	if (method == "GET") return GET;
// 	else if (method == "POST") return POST;
// 	else if (method == "DELETE") return DELETE;
// 	else if (method == "PUT") return PUT;
// 	else if (method == "HEAD") return HEAD;
// 	else if (method == "OPTIONS") return OPTIONS;
// 	else if (method == "PATCH") return PATCH;
// 	else return INVALID;
// 	*/
// 	loc.accepted_methods.size();
// }

// static void parseRoot(std::string &line, int &line_n, s_location &loc)
// {
// }

// static void parseAutoIndex(std::string &line, int &line_n, s_location &loc)
// {
// }

// static void parseIndex(std::string &line, int &line_n, s_location &loc)
// {
// }

// static void parseUploadDir(std::string &line, int &line_n, s_location &loc)
// {
// }

// static void parseClientMaxBodySize(std::string &line, int &line_n, s_location &loc)
// {
// }

void findLocationDirective(std::string &line, int &line_n, s_location &loc)
{
	(void)line;
	(void)line_n;
	(void)loc;
}

/*
allowed_methods:
GET / POST / DELETE
eHttpMethod

root
path

autoindex
on / off

index
index.html index.htm
if neither file is found server may return 403 Forbidden or 404 Not Found

upload_dir
path

client_max_body_size
same as other max_body_size parser

return -- redirects request from old route to new-route wiht a 301 status
301 /new-route

cgi_ext
.php / .pl / .py

-> only 1 per location possible

cgi_path -> allow only 1
path

*/