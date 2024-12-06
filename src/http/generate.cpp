/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/06 14:53:38 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief generates the httpResponse based on the information inside _request
 * @note remove printing later
 */
s_httpSend httpHandler::generateResponse(void)
{
	auto it = _request.headers.find(eRequestHeader::Connection);
	if (it != _request.headers.end())
	{
		if (it->second == "close")
			_response.keepalive = false;
	}
	callMethod();
	// CGI generate its own HTTP
	if (_statusCode > eHttpStatusCode::Accepted)
		return (writeResponse());
	return (writeResponse());
}

/**
 * @brief Builds the response based on the information in s_response;
 */
s_httpSend httpHandler::writeResponse(void)
{
	if (_response.cgi == true | _response.readFile == true)
	{
		return {"", _response.keepalive, _response.readFile, _response.readFd, _response.cgi, _response.pid};
	}
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
	{
		std::stringstream responseStream;
		// status line
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " "
					   << statusMessages.at(_statusCode) << "\r\n";
		// headers
		if (_response.keepalive)
			_response.headers[eResponseHeader::Connection] = "keep-alive";
		else
			_response.headers[eResponseHeader::Connection] = "close";
		for (const auto &header : _response.headers)
		{
			responseStream << responseHeaderToString(header.first) << header.second << "\r\n";
		}
		// break between header and body:
		responseStream << "\r\n";
		// body
		if (_response.readFile == false && _response.cgi == false)
		{
			responseStream << _response.body.str();
		}
		s_httpSend response = {responseStream.str(), _response.keepalive, _response.readFile, _response.readFd, _response.cgi, _response.pid};
		return (response);
	}
	else
	{
		_statusCode = eHttpStatusCode::BadRequest;
		std::string message = "Bad request";
		std::ostringstream responseStream;
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " " << message << "\r\n"
					   << "Content-Type: text/plain\r\n"
					   << "Content-Length: " << message.size() << "\r\n"
					   << "Connection: close\r\n"
					   << "\r\n"
					   << message;
		s_httpSend response = {responseStream.str(), _response.keepalive, false, -1, false, -1};
		return (response);
	}
}

void httpHandler::callMethod()
{
	if (_request.method == eHttpMethod::GET)
		stdGet();
	else if (_request.method == eHttpMethod::POST)
		stdPost();
	else if (_request.method == eHttpMethod::DELETE)
		stdDelete();
	return;
}