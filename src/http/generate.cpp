/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/25 10:08:27 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

#define LINEBREAK "\r\n"
const std::string CONTENT_TYPE_TEXT = "Content-Type: text/plain";
const std::string CONTENT_TYPE_HTML = "Content-Type: text/html";
const std::string CONTENT_TYPE_JSON = "Content-Type: application/json";
const std::string CONTENT_TYPE_XML = "Content-Type: application/xml";
const std::string CONTENT_TYPE_JAVASCRIPT = "Content-Type: application/javascript";
const std::string CONTENT_TYPE_CSS = "Content-Type: text/css";
const std::string CONTENT_TYPE_OCTET_STREAM = "Content-Type: application/octet-stream";

std::string httpHandler::generateResponse()
{
	if (_request.statusCode != eHttpStatusCode::OK)
		return (writeResponse());

	std::string response;
	if (_request.cgi == true)
		response = cgiRequest();
	else
		response = stdRequest();
	return response;
}

/**
 * @brief writes a simple response in case the Parser returned a statusCode;
 * @note update to write firstline + stored ResponseHeaders + body
 */
std::string httpHandler::writeResponse(void)
{
	std::string message;

	auto it = statusMessages.find(_request.statusCode);
	if (it != statusMessages.end())
		message = it->second;
	else
	{
		message = "Bad request";
		_request.statusCode = eHttpStatusCode::BadRequest;
	}
	std::ostringstream response;
	response << "HTTP/1.1 " << static_cast<int>(_request.statusCode) << " " << message << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << message.size() << "\r\n"
			 << "Connection: close\r\n"
			 << "\r\n"
			 << message;
	return (response.str());
}
