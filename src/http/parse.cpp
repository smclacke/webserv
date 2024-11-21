/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parse.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/21 15:42:00 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include "../../include/web.hpp"

/* member functions */

/**
 * @brief Parses the https request and stores it in the httpHandler::_request
 */
void httpHandler::parseRequest(const std::string &httpRequest)
{
	std::istringstream ss(httpRequest);

	// Get the request line
	parseRequestLine(ss);
	if (_request.statusCode != eHttpStatusCode::OK)
		return;
	parseHeaders(ss);
	if (_request.statusCode != eHttpStatusCode::OK)
		return;
	parseBody(ss);
	return;
}

/* private functions */

/**
 * @brief Parses the request line of the HTTP request
 */
void httpHandler::parseRequestLine(std::istringstream &ss)
{
	std::string requestLine;

	// Get the request line
	if (!std::getline(ss, requestLine))
	{
		std::cerr << "Failed to read request line" << std::endl;
		_request.statusCode = eHttpStatusCode::BadRequest;
		return;
	}
	/* parse request line */
	std::istringstream requestss(requestLine);
	std::string methodstring, version;
	if (!(requestss >> methodstring >> _request.uri >> version))
	{
		std::cerr << "Invalid request line format" << std::endl;
		_request.statusCode = eHttpStatusCode::BadRequest;
		return;
	}
	// check METHOD
	_request.method = _server.allowedHttpMethod(methodstring);
	if (_request.method == eHttpMethod::INVALID) // method check
	{
		std::cerr << "HTTP method not allowed: " << methodstring << std::endl;
		_request.statusCode = eHttpStatusCode::MethodNotAllowed;
		return;
	}
	// Check if URI is too long
	const size_t MAX_URI_LENGTH = 2048; // Example limit, adjust as needed
	if (_request.uri.length() > MAX_URI_LENGTH)
	{
		std::cerr << "URI too long: " << _request.uri << std::endl;
		_request.statusCode = eHttpStatusCode::URITooLong;
		return;
	}
	// check version
	if (version != "HTTP/1.1")
	{
		std::cerr << "HTTP version not supported: " << version << std::endl;
		_request.statusCode = eHttpStatusCode::HTTPVersionNotSupported;
		return;
	}

	// URI match against location
	_request.loc = findLongestPrefixMatch(_request.uri, _server.getLocation());
	size_t pos = _request.uri.find_last_of(".");
	if (pos != std::string::npos)
	{
		std::string extension = _request.uri.substr(pos);
		if (extension == _request.loc.cgi_ext)
			_request.cgi = true;
	}
	if (_request.cgi == true)
	{
		std::cout << "This request is a cgi request" << std::endl;
		if (!_request.loc.root.empty())
			_request.path = "." + _request.loc.root + _request.uri;
		else
			_request.path = "." + _server.getRoot() + _request.uri;
		if (!std::filesystem::exists(_request.path))
		{
			std::cerr << "CGI script not found at path: " << _request.path << std::endl;
			_request.statusCode = eHttpStatusCode::NotFound;
			return;
		}
	}
	else
	{
		if (!_request.loc.root.empty())
			_request.path = "." + _request.loc.root + _request.uri;
		else
			_request.path = "." + _server.getRoot() + _request.uri;
		if (!std::filesystem::exists(_request.path))
		{
			std::cerr << "Resource not found at path: " << _request.path << std::endl;
			_request.statusCode = eHttpStatusCode::NotFound;
			return;
		}
	}
}

/**
 * @brief Parses the headers of the HTTP request
 */
void httpHandler::parseHeaders(std::istringstream &ss)
{
	std::string header;
	std::string key, value;
	while (std::getline(ss, header) && !header.empty() && header != "\r")
	{
		if (header.back() == '\r')
			header.pop_back();
		std::istringstream split(header);
		getline(split, key, ':');
		getline(split >> std::ws, value);
		eRequestHeader headerType = toEHeader(key);
		if (headerType == eRequestHeader::Invalid)
		{
			std::cerr << "Invalid header key: " << key << std::endl;
			_request.statusCode = eHttpStatusCode::BadRequest;
			return;
		}
		_request.headers[headerType] = value;
	}
}
