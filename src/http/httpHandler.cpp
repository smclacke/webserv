/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 17:21:12 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/11 18:13:23 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/epoll.hpp"
#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"

/* constructor and deconstructor */

httpHandler::httpHandler(Server &server, Epoll &epoll) : _server(server), _epoll(epoll)
{
	_statusCode = eHttpStatusCode::OK;
	// request
	_request.keepReading = true;
	_request.method = eHttpMethod::INVALID;
	_request.uri.clear();
	_request.path.clear();
	_request.headers.clear();
	_request.uriEncoded = false;
	_request.uriQuery.clear();
	_request.head.clear();
	_request.headCompleted = false;
	_request.body.content.clear();
	_request.body.contentType = eContentType::error;
	_request.body.contentLen = 0;
	_request.body.nextChunkSize = 0;
	_request.body.totalChunked = 0;
	_request.body.formDelimiter.clear();
	//	response
	_response.headers.clear();
	_response.body.clear();
	_response.keepalive = true;
	_response.readFile = false;
	_response.cgi = false;
	_response.readFd = -1;
	// cgi
	_cgi.cgiIN[0] = -1;
	_cgi.cgiIN[1] = -1;
	_cgi.cgiOUT[0] = -1;
	_cgi.cgiOUT[1] = -1;
	_cgi.close = false;
	_cgi.state = cgiState::BEGIN;
}

httpHandler::~httpHandler(void)
{
	for (char *envVar : _cgi.env)
	{
		free(envVar);
	}
	_cgi.clearCgi();
}

void httpHandler::clearHandler(void)
{
	_statusCode = eHttpStatusCode::OK;
	// request
	_request.keepReading = true;
	_request.method = eHttpMethod::INVALID;
	_request.uri.clear();
	_request.path.clear();
	_request.headers.clear();
	_request.uriEncoded = false;
	_request.uriQuery.clear();
	_request.head.clear();
	_request.headCompleted = false;
	_request.body.content.clear();
	_request.body.contentType = eContentType::error;
	_request.body.contentLen = 0;
	_request.body.nextChunkSize = 0;
	_request.body.totalChunked = 0;
	_request.body.formDelimiter.clear();
	//	response
	_response.headers.clear();
	_response.body.clear();
	_response.keepalive = true;
	_response.readFile = false;
	_response.cgi = false;
	_response.readFd = -1;
	// cgi
	for (char *envVar : _cgi.env)
	{
		free(envVar);
	}
	_cgi.clearCgi();
}

/* utils */

/**
 * @brief finds the longestPrefix match for matching the URI against the location.path in the vector of locations
 */
std::optional<s_location> httpHandler::findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks)
{
	std::optional<s_location> longestMatch = std::nullopt;

	for (const auto &location : locationBlocks)
	{
		// Check if the location is a prefix of the request URI
		if (requestUri.find(location.path) == 0)
		{ // Check if the requestUri starts with location
			if (!longestMatch || location.path.length() > longestMatch->path.length())
			{
				longestMatch = location;
			}
		}
	}
	return longestMatch;
}

/**
 * @brief finds the corresponding value to a headerKey
 * @return returns the value of the header or std::nullopt if header doesnt exist
 */
std::optional<std::string> httpHandler::findHeaderValue(const s_request &request, eRequestHeader headerKey)
{
	auto it = request.headers.find(headerKey);
	if (it != request.headers.end())
	{
		return it->second;
	}
	return std::nullopt;
}

/**
 * @brief returns a eRequestHeader based on the header string provided"
 * @return header if found, if not found eRequestHeader::Invalid
 */
eRequestHeader httpHandler::toEHeader(const std::string &header)
{
	auto it = headerMap.find(header);
	return it != headerMap.end() ? it->second : eRequestHeader::Invalid;
}

/**
 * @brief Transforms the enum to a string for printing
 * @warning slower look up table, use for testing only, not for live Server
 * @note add map for Enum - String if this function requires constant use
 */
std::string httpHandler::EheaderToString(const eRequestHeader &header)
{
	// Reverse the existing headerMap to create a lookup for eRequestHeader to string
	static std::unordered_map<eRequestHeader, std::string> requestHeaderToString;
	if (requestHeaderToString.empty())
	{
		for (const auto &pair : headerMap)
		{
			requestHeaderToString[pair.second] = pair.first;
		}
	}

	auto it = requestHeaderToString.find(header);
	return it != requestHeaderToString.end() ? it->second : "Unknown Header";
}

/**
 * @brief returns a string corresponding to the header
 * @return "header: " or if the eResponseHeader isnt mapped an empty std::string
 */
std::string httpHandler::responseHeaderToString(const eResponseHeader &header)
{
	static const std::unordered_map<eResponseHeader, std::string> responseHeaderMap = {
		{eResponseHeader::ContentType, "Content-Type: "},
		{eResponseHeader::ContentLength, "Content-Length: "},
		{eResponseHeader::ContentEncoding, "Content-Encoding: "},
		{eResponseHeader::Connection, "Connection: "},
		{eResponseHeader::SetCookie, "Set-Cookie: "},
		{eResponseHeader::CacheControl, "Cache-Control: "},
		{eResponseHeader::Expires, "Expires: "},
		{eResponseHeader::ETag, "ETag: "},
		{eResponseHeader::LastModified, "Last-Modified: "},
		{eResponseHeader::Location, "Location: "},
		{eResponseHeader::WWWAuthenticate, "WWW-Authenticate: "},
		{eResponseHeader::RetryAfter, "Retry-After: "},
		{eResponseHeader::AccessControlAllowOrigin, "Access-Control-Allow-Origin: "},
		{eResponseHeader::StrictTransportSecurity, "Strict-Transport-Security: "},
		{eResponseHeader::Vary, "Vary: "},
		{eResponseHeader::Server, "Server: "},
		{eResponseHeader::ContentDisposition, "Content-Disposition: "}};
	auto it = responseHeaderMap.find(header);
	return it != responseHeaderMap.end() ? it->second : "";
}

void httpHandler::setErrorResponse(eHttpStatusCode code, std::string msg)
{
	_statusCode = code;
	_response.body.clear();
	_response.body << msg;
}

/**
 * @brief adds the buffer to the request body
 * @return true if the epoll should continue reading
 * false if the epoll should stop reading
 *
 * determined by"
 * end of ContentLength / Chunked=0/r/n/ / wwwFormDelimiter
 *
 */
void httpHandler::addStringBuffer(std::string &buffer)
{
	std::cout << "==buffer==" << buffer << std::endl;
	if (!_request.headCompleted)
	{
		size_t pos = buffer.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			_request.head << buffer.substr(0, pos + 4);
			_request.headCompleted = true;
			parseHead();
			if (_statusCode > eHttpStatusCode::Accepted)
			{
				_request.keepReading = false;
				return;
			}
			setContent();
			if (_request.body.contentType == eContentType::error || _request.body.contentType == eContentType::noContent)
			{
				_request.keepReading = false;
				return;
			}
			if (pos + 4 < buffer.size())
			{
				std::string buf = buffer.substr(pos + 4);
				addToBody(buf);
			}
			return;
		}
		else
		{
			_request.head << buffer;
		}
	}
	else
	{
		if (_request.body.contentType == eContentType::error || _request.body.contentType == eContentType::noContent)
		{
			setErrorResponse(eHttpStatusCode::InternalServerError, "reached unexpected point");
			_request.keepReading = false;
			return;
		}
		addToBody(buffer);
	}
}

/**
 * @brief returns either READ_BUFFER_SIZE or the size of next chunk
 */
size_t httpHandler::getReadSize(void) const
{
	if (_request.body.contentType == eContentType::chunked)
		return (_request.body.nextChunkSize);
	return (READ_BUFFER_SIZE);
}

bool httpHandler::getKeepReading(void) const
{
	return (_request.keepReading);
}

/* CGI :) */
void s_cgi::clearCgi(void)
{
	env.clear();
	scriptname.clear();
	pid = -1;
	close = false;
	state = cgiState::BEGIN;	
	write_offset = 0;
	output = false;
	input.clear();
	closeAllPipes();
}

void		s_cgi::closeAllPipes(void)
{
	if (cgiIN[0] != -1)
	{
		protectedClose(cgiIN[0]);
		cgiIN[0] = -1;	
	}
	if (cgiIN[1] != -1)
	{
		protectedClose(cgiIN[1]);
		cgiIN[1] = -1;	
	}
	if (cgiOUT[0] != -1)
	{
		protectedClose(cgiOUT[0]);
		cgiOUT[0] = -1;	
	}
	if (cgiOUT[1] != -1)
	{
		protectedClose(cgiOUT[1]);
		cgiOUT[1] = -1;	
	}
}

s_cgi	httpHandler::getCGI()
{
	_cgi.input = _request.body.content.str();
	return this->_cgi;
}
