/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parse.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 15:10:37 by jde-baai      ########   odam.nl         */
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
void httpHandler::parseRequest(std::stringstream &httpRequest)
{
	parseRequestLine(httpRequest);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	parseHeaders(httpRequest);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	checkUriPath();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	std::string remainingData;
	std::getline(httpRequest, remainingData, '\0');
	// Check if the remaining data is not just the end of headers
	if (!remainingData.empty() && remainingData != "\r\n\r\n")
	{
		// Place the remaining data back into the stream
		httpRequest.clear();			// Clear any error flags
		httpRequest.str(remainingData); // Reset the stream with the remaining data
		httpRequest.seekg(0);			// Reset the position to the beginning of the stream
		parseBody(httpRequest);
	}
	return;
}

/* private functions */

/**
 * @brief Parses the request line of the HTTP request
 */
void httpHandler::parseRequestLine(std::stringstream &ss)
{
	std::string requestLine;

	// Get the request line
	if (!std::getline(ss, requestLine))
	{
		std::cerr << "Failed to read request line" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Failed to read request line");
	}
	/* parse request line */
	std::istringstream requestss(requestLine);
	std::string methodstring, version;
	if (!(requestss >> methodstring >> _request.uri >> version))
	{
		std::cerr << "Invalid request line format" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Invalid request line format");
	}
	// check METHOD
	_request.method = _server.allowedHttpMethod(methodstring);
	if (_request.method == eHttpMethod::INVALID) // method check
	{
		std::cerr << "HTTP method not allowed: " << methodstring << std::endl;
		return setErrorResponse(eHttpStatusCode::MethodNotAllowed, "HTTP method not allowed: " + methodstring);
	}
	// Check if URI is too long
	const size_t MAX_URI_LENGTH = 2048; // Example limit, adjust as needed
	if (_request.uri.length() > MAX_URI_LENGTH)
	{
		std::cerr << "URI too long: " << _request.uri << std::endl;
		return setErrorResponse(eHttpStatusCode::URITooLong, "URI too long: " + _request.uri);
	}
	// check version
	if (version != "HTTP/1.1")
	{
		std::cerr << "HTTP version not supported: " << version << std::endl;
		return setErrorResponse(eHttpStatusCode::HTTPVersionNotSupported, "HTTP version not supported: " + version);
	}

	// URI match against location
	std::optional<s_location> optLoc = findLongestPrefixMatch(_request.uri, _server.getLocation());
	if (!optLoc.has_value())
	{
		std::cerr << "No Matching location for URI: " << _request.uri << std::endl;
		return setErrorResponse(eHttpStatusCode::NotFound, "No Matching location for URI: " + _request.uri);
	}
	_request.loc = optLoc.value();
}

void httpHandler::checkUriPath(void)
{
	auto contentTypeIt = _request.headers.find(eRequestHeader::ContentType);
	if (contentTypeIt != _request.headers.end())
	{
		const std::string &contentType = contentTypeIt->second;
		if (contentType == "application/x-www-form-urlencoded")
			_request.uriEncoded = true;
		if (_request.uri.find("?") != std::string::npos)
		{
			std::string uripath = _request.uri.substr(0, _request.uri.find("?"));
			std::string path;
			if (_request.loc.root.empty())
				std::string path = "." + _server.getRoot() + uripath;
			else
				std::string path = "." + _request.loc.root + uripath;
			if (!std::filesystem::exists(path))
				return setErrorResponse(eHttpStatusCode::NotFound, "Path doesnt exist: " + path);
			return;
		}
		else
			return setErrorResponse(eHttpStatusCode::BadRequest, "Expected query parameters in URI");
	}
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
			return setErrorResponse(eHttpStatusCode::NotFound, "CGI script not found at path: " + _request.path);
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
			return setErrorResponse(eHttpStatusCode::NotFound, "Resource not found at path: " + _request.path);
		}
	}
}

/**
 * @brief Parses the headers of the HTTP request
 */
void httpHandler::parseHeaders(std::stringstream &ss)
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
			return setErrorResponse(eHttpStatusCode::BadRequest, "Invalid header key: " + key);
		}
		if (headerType == eRequestHeader::Connection)
		{
			if (key != "keep-alive" && key != "close")
				return setErrorResponse(eHttpStatusCode::NotImplemented, "Connection type not implemented: " + key);
		}
		_request.headers[headerType] = value;
	}
}
