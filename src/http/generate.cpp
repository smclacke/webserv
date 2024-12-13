/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/13 09:18:36 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief generates the httpResponse based on the information inside _request
 * @note remove printing later
 */
s_httpSend httpHandler::generateResponse(void)
{
	std::cout << "\n***********************\n";
	std::cout << "generate response called for :\n"
			  << _request.path << "\n"
			  << "METHOD = " << httpMethodToStringFunc(_request.method) << "\n"
			  << "\n***********************\n";
	auto it = _request.headers.find(eRequestHeader::Connection);
	if (it != _request.headers.end())
	{
		if (it->second == "close")
			_response.keepalive = false;
	}
	if (_statusCode > eHttpStatusCode::Accepted)
		return (writeResponse());
	callMethod();
	if (_response.readFile)
		std::cout << "\n****** read_file is true ****" << std::endl;
	else
		std::cout << "\n****** read_file is false ****" << std::endl;
	return (writeResponse());
}

static s_httpSend internalError(void);

/**
 * @brief Builds the response based on the information in s_response;
 */
s_httpSend httpHandler::writeResponse(void)
{
	if (_response.cgi)
	{
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
		std::cout << "Response = " << response.msg << std::endl;
		return (response);
	}
	else
	{
		return (internalError());
	}
}

static s_httpSend internalError(void)
{
	eHttpStatusCode statusCode = eHttpStatusCode::InternalServerError;
	std::string message = "Unknown response type";
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << message << "\r\n"
				   << "Content-Type: text/plain\r\n"
				   << "Content-Length: " << message.size() << "\r\n"
				   << "Connection: close\r\n"
				   << "\r\n"
				   << message;
	s_httpSend response = {responseStream.str(), true, false, -1, false};
	return (response);
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
