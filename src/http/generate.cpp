/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/31 15:57:34 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief generates the httpResponse based on the information inside _request
 * @note remove printing later
 */
s_httpSend httpHandler::generateResponse(void)
{
	_server.logMessage("generateResponse(): HTTP request received: ");
	_server.logMessage(_request.head.str());
	_server.logMessage(_request.body.content.str());
	auto it = _request.headers.find(eRequestHeader::Connection);
	if (it != _request.headers.end())
	{
		if (it->second == "close")
			_response.keepalive = false;
	}
	if (_statusCode > eHttpStatusCode::Accepted)
		return (writeResponse());
	callMethod();
	return (writeResponse());
}

/**
 * @brief Builds the response based on the information in s_response;
 */
s_httpSend httpHandler::writeResponse(void)
{
	if (_response.cgi)
	{
		_server.logMessage("writeResponse(): response is cgi script");
		s_httpSend response = {"", _response.keepalive, _response.readFile, _response.readFd, _response.cgi};
		return (response);
	}
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
	{
		std::stringstream responseStream;
		// status line
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " "
					   << statusMessages.at(_statusCode) << "\r\n";
		// check if there is a related errorPage
		if (static_cast<int>(_statusCode) > 308)
		{
			auto optPage = _server.findErrorPage(static_cast<int>(_statusCode));
			if (optPage.has_value())
			{
				CallErrorPage(optPage.value().path);
			}
		}
		/* headers */
		// set keepalive
		if (_response.keepalive)
			_response.headers[eResponseHeader::Connection] = "keep-alive";
		else
			_response.headers[eResponseHeader::Connection] = "close";
		// Content-Length check
		if (_response.headers.find(eResponseHeader::ContentLength) == _response.headers.end())
		{
			_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
		}
		for (const auto &header : _response.headers)
		{
			responseStream << responseHeaderToString(header.first) << header.second << "\r\n";
		}
		// break between header and body:
		responseStream << "\r\n";
		/* body */
		if (_response.readFile == false)
			responseStream << _response.body.str();
		s_httpSend response = {responseStream.str(), _response.keepalive, _response.readFile, _response.readFd, _response.cgi};
		_server.logMessage("writeResponse(): response: ");
		_server.logMessage(response.msg);
		return (response);
	}
	else
	{
		s_httpSend response = {internalError("Unknown response type"), false, false, -1, false};
		_server.logMessage("writeResponse(): response: ");
		_server.logMessage(response.msg);
		return (response);
	}
}

std::string internalError(std::string msg)
{
	eHttpStatusCode statusCode = eHttpStatusCode::InternalServerError;
	std::string message = msg;
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << message << "\r\n"
				   << "Content-Type: text/plain\r\n"
				   << "Content-Length: " << message.size() << "\r\n"
				   << "Connection: close\r\n"
				   << "\r\n"
				   << message;
	return (responseStream.str());
}

void httpHandler::callMethod(void)
{
	if (_request.method == eHttpMethod::GET)
		getMethod();
	else if (_request.method == eHttpMethod::POST)
		postMethod();
	else if (_request.method == eHttpMethod::DELETE)
		stdDelete();
	return;
}
