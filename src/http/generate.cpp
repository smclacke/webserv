/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/25 11:03:32 by jde-baai      ########   odam.nl         */
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
	std::cout << "By generateResponse(), incoming request is as follows:\n";
	std::cout << "Status code: " << static_cast<int>(_statusCode) << std::endl;
	std::cout << "Method: " << HttpMethodToString.at(_request.method) << std::endl;
	std::cout << "URI: " << _request.uri << std::endl;
	if (!_request.path.empty())
		std::cout << "Location: " << _request.loc.path << std::endl;
	std::cout << "Path: " << _request.path << std::endl;
	for (const auto &header : _request.headers)
		std::cout << "Header: " << EheaderToString(header.first) << " - " << header.second << std::endl;
	std::cout << "Body: " << _request.body.str() << std::endl;
	std::cout << "CGI: " << _request.cgi << std::endl;

	if (_statusCode != eHttpStatusCode::OK)
		return (writeResponse());

	if (_request.cgi == true)
		cgiRequest();
	else
		stdRequest();
	return writeResponse();
}

/**
 * @brief writes a simple response in case the Parser returned a statusCode;
 */
std::string httpHandler::writeResponse(void)
{
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
	{
		std::stringstream responseStream;
		// status line
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " "
					   << statusMessages.at(_statusCode) << "\r\n";
		// headers
		for (const auto &header : _response.headers)
		{
			responseStream << responseHeaderToString(header.first) << header.second << "\r\n";
		}
		// End headers section
		responseStream << "\r\n";
		// body
		responseStream << _response.body.str();
		return responseStream.str();
	}
	else
	{
		_statusCode = eHttpStatusCode::BadRequest;
		std::string message = "Bad request";
		std::ostringstream response;
		response << "HTTP/1.1 " << static_cast<int>(_statusCode) << " " << message << "\r\n"
				 << "Content-Type: text/plain\r\n"
				 << "Content-Length: " << message.size() << "\r\n"
				 << "Connection: close\r\n"
				 << "\r\n"
				 << message;
		return (response.str());
	}
}
