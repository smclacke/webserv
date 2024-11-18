/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/15 17:44:41 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

std::string httpHandler::stdRequest(void)
{
	std::cout << "It is a standard request with these inputs:" << std::endl;
	std::cout << "Method: " << HttpMethodToString.at(_request.method) << std::endl;
	std::cout << "URI: " << _request.uri << std::endl;
	std::cout << "Location: " << _request.loc.path << std::endl;
	std::cout << "Path: " << _request.path << std::endl;
	for (const auto &header : _request.headers)
		std::cout << "Header: " << header.first << " - " << header.second << std::endl;
	std::cout << "Body: " << _request.body << std::endl;
	std::cout << "CGI: " << _request.cgi << std::endl;
	return ("response");
}