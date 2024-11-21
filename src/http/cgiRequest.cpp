/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgiRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 14:51:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/21 13:55:09 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

std::string httpHandler::cgiRequest(void)
{
	std::cout << "It is a CGI request with these inputs:" << std::endl;
	std::cout << "Method: " << HttpMethodToString.at(_request.method) << std::endl;
	std::cout << "URI: " << _request.uri << std::endl;
	std::cout << "Location: " << _request.loc.path << std::endl;
	std::cout << "Path: " << _request.path << std::endl;
	for (const auto &header : _request.headers)
		std::cout << "Header: " << EheaderToString(header.first) << " - " << header.second << std::endl;
	std::cout << "Body: " << _request.body.str() << std::endl;
	std::cout << "CGI: " << _request.cgi << std::endl;
	return (writeResponse());
}