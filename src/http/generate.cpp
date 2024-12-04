/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/04 21:48:48 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief generates the httpResponse based on the information inside _request
 * @note remove printing later
 */
s_httpSend httpHandler::generateResponse(void)
{
	/* printing to be removed later */
	//std::cout << "By generateResponse(), incoming request is as follows:\n";
	//std::cout << "Status code: " << static_cast<int>(_statusCode) << std::endl;
	//std::cout << "Method: " << HttpMethodToString.at(_request.method) << std::endl;
	//std::cout << "URI: " << _request.uri << std::endl;
	//if (!_request.path.empty())
	//	std::cout << "Location: " << _request.loc.path << std::endl;
	//std::cout << "Path: " << _request.path << std::endl;
	//for (const auto &header : _request.headers)
	//	std::cout << "Header: " << EheaderToString(header.first) << " - " << header.second << std::endl;
	//std::cout << "Body: " << _request.body.str() << std::endl;

	bool keepalive = true;
	auto it = _request.headers.find(eRequestHeader::Connection);
	if (it != _request.headers.end())
	{
		if (it->second == "close")
			keepalive = false;
	}

	if (_statusCode > eHttpStatusCode::Accepted)
		return (writeResponse(keepalive));
	callMethod();
	return writeResponse(keepalive);
}

/**
 * @brief Builds the response based on the information in s_response;
 */
s_httpSend httpHandler::writeResponse(bool keepalive)
{
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
	{
		std::cout << "BODY = " << _response.body.str() << std::endl;
		std::stringstream responseStream;
		// status line
		responseStream << "HTTP/1.1 " << static_cast<int>(_statusCode) << " "
					   << statusMessages.at(_statusCode) << "\r\n";
		if (!_response.body.str().empty())
		{
			if (_response.headers.find(eResponseHeader::ContentLength) == _response.headers.end())
				_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
		}
		// headers
		for (const auto &header : _response.headers)
		{
			responseStream << responseHeaderToString(header.first) << header.second << "\r\n";
		}
		// End headers section
		responseStream << "\r\n";
		// body
		if (_response.readFile == false && _response.cgi == false)
		{
			responseStream << _response.body.str();
		}
		s_httpSend response = {responseStream.str(), keepalive, _response.filepath, _response.readFile, _response.readFd, _response.pid};
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
		s_httpSend response = {responseStream.str(), keepalive, "", false, -1, -1};
		return (response);
	}
}

void httpHandler::callMethod(void)
{
	if (_request.method == eHttpMethod::GET)
		stdGet();
	else if (_request.method == eHttpMethod::POST)
		stdPost();
	else if (_request.method == eHttpMethod::DELETE)
		stdDelete();
	return;
}