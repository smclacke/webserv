/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parse.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/20 14:54:12 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include "../../include/web.hpp"

/*
Request line - "method request_URI HTTP version"
Host line - "Host: domain name of the server"
User-Agent - "Information about the client doing the request"
Content-type - "Content-Type: type of media"
Content-Length - "Content-Length: number"
Body - "Name=John&age=30"

Request line -> must always be on top
Headers -> can appear in any order
Body -> after headers and appears after a blank line inbetween

*/

/* member functions */

std::string httpHandler::parseRequest(const std::string &httpRequest)
{
	std::istringstream ss(httpRequest);

	// Get the request line
	parseRequestLine(ss);
	if (_request.statusCode != eHttpStatusCode::NotSet)
		return (generateHttpResponse(_request.statusCode));
	parseHeaders(ss);
	if (_request.statusCode != eHttpStatusCode::NotSet)
		return (generateHttpResponse(_request.statusCode));
	parseBody(ss);
	if (_request.statusCode != eHttpStatusCode::NotSet)
		return (generateHttpResponse(_request.statusCode));
	if (_request.cgi == true)
		cgiRequest();
	else
		stdRequest();
	return (generateHttpResponse(_request.statusCode));
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
		_request.statusCode = eHttpStatusCode::BadRequest;
		return;
	}
	// check version
	if (version != "HTTP/1.1")
	{
		_request.statusCode = eHttpStatusCode::HTTPVersionNotSupported;
		return;
	}
	// check METHOD
	_request.method = _server.allowedHttpMethod(methodstring);
	if (_request.method == eHttpMethod::INVALID) // method check
	{
		_request.statusCode = eHttpStatusCode::MethodNotAllowed;
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
		if (headerType == Invalid)
		{
			_request.statusCode = eHttpStatusCode::BadRequest;
			return;
		}
		else if (headerType > 5)
		{
			_request.statusCode = eHttpStatusCode::NotImplemented;
			return;
		}
		_request.headers[headerType] = value;
	}
}

/**
 * @brief Parses the body of the HTTP request
 */
void httpHandler::parseBody(std::istringstream &ss)
{
	std::optional<std::string> transferEncoding = findHeaderValue(_request, eRequestHeader::TransferEncoding);
	std::optional<std::string> contentEncoding = findHeaderValue(_request, eRequestHeader::ContentEncoding);
	std::optional<std::string> contentLength = findHeaderValue(_request, eRequestHeader::ContentLength);

	if (transferEncoding.has_value())
	{
		if (transferEncoding.value() == "chunked")
		{
			parseChunkedBody(ss);
		}
		else if (transferEncoding.value() == "identity" && contentLength.has_value())
		{
			parseFixedLengthBody(ss, std::stoul(contentLength.value()));
		}
		else
		{
			std::cerr << "Unsupported Transfer-Encoding: " << transferEncoding.value() << std::endl;
			_request.statusCode = eHttpStatusCode::NotImplemented;
			return;
		}
	}
	else if (contentLength.has_value())
	{
		parseFixedLengthBody(ss, std::stoi(contentLength.value()));
	}
	else
	{
		std::cerr << "No Content-Length or Transfer-Encoding header present" << std::endl;
		_request.statusCode = eHttpStatusCode::LengthRequired;
		return;
	}
	if (contentEncoding.has_value())
	{
		decodeContentEncoding(_request.body, contentEncoding.value());
	}
}

/**
 * @brief Reads a chunked body from the HTTP request
 */
void httpHandler::parseChunkedBody(std::istringstream &ss)
{
	// Implement chunked body parsing logic here
	// Handle chunked transfer encoding
	std::string chunkSizeLine;
	while (std::getline(ss, chunkSizeLine))
	{
		if (!chunkSizeLine.empty() && chunkSizeLine.back() == '\r')
			chunkSizeLine.pop_back();

		std::istringstream chunkSizeStream(chunkSizeLine);
		size_t chunkSize;
		chunkSizeStream >> std::hex >> chunkSize;

		if (chunkSize == 0)
			break;

		std::string chunkData(chunkSize, '\0');
		ss.read(&chunkData[0], chunkSize);
		_request.body << chunkData;

		// Read the trailing \r\n after the chunk data
		std::getline(ss, chunkSizeLine);
	}
}

/**
 * @note do something with length
 */
void httpHandler::parseFixedLengthBody(std::istringstream &ss, size_t length)
{
	(void) length;
	// Implement fixed-length body parsing logic here
	std::string bodyLine;
	while (std::getline(ss, bodyLine) && bodyLine != "0\r")
	{
		if (!bodyLine.empty() && bodyLine.back() == '\r')
			bodyLine.pop_back();
		_request.body << bodyLine << "\n";
	}
	std::string bodyStr = _request.body.str();
	if (!bodyStr.empty() && bodyStr.back() == '\n')
	{
		bodyStr.pop_back();
		_request.body.str(bodyStr);
	}
}

void httpHandler::decodeContentEncoding(std::stringstream &body, const std::string &encoding)
{
	(void) body;
	if (encoding == "identity")
	{
		return;
	}
	else if (encoding == "gzip")
	{
		// Implement gzip decoding logic here
	}
	else if (encoding == "deflate")
	{
		// Implement deflate decoding logic here
	}
	else if (encoding == "br")
	{
		// Implement Brotli decoding logic here
	}
	else
	{
		std::cerr << "Unsupported Content-Encoding: " << encoding << std::endl;
		_request.statusCode = eHttpStatusCode::NotImplemented;
	}
}