/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 17:21:12 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/20 16:39:50 by smclacke      ########   odam.nl         */
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
	_request.cgiReq = false;
	_request.path.clear();
	_request.headers.clear();
	_request.uriEncoded = false;
	_request.uriQuery.clear();
	resetStringStream(_request.head);
	_request.headCompleted = false;
	resetStringStream(_request.body.content);
	_request.body.contentType = eContentType::error;
	_request.body.contentLen = 0;
	_request.body.chunked = false;
	_request.body.nextChunkSize = 0;
	_request.body.totalChunked = 0;
	_request.body.formDelimiter.clear();
	_request.bufferVec.clear();
	_request.body.contentVec.clear();
	//	response
	_response.headers.clear();
	resetStringStream(_response.body);
	_response.keepalive = true;
	_response.readFile = false;
	_response.cgi = false;
	_response.readFd = -1;
	// cgi
	_cgi.cgiIN[0] = -1;
	_cgi.cgiIN[1] = -1;
	_cgi.cgiOUT[0] = -1;
	_cgi.cgiOUT[1] = -1;
	_cgi.state = cgiState::BEGIN;
	_cgi.htmlOutput = false;
}

httpHandler::~httpHandler(void)
{
}

void httpHandler::clearHandler(void)
{
	_statusCode = eHttpStatusCode::OK;
	// request
	_request.keepReading = true;
	_request.method = eHttpMethod::INVALID;
	_request.uri.clear();
	_request.cgiReq = false;
	_request.path.clear();
	_request.headers.clear();
	_request.uriEncoded = false;
	_request.uriQuery.clear();
	resetStringStream(_request.head);
	_request.headCompleted = false;
	resetStringStream(_request.body.content);
	_request.body.contentType = eContentType::error;
	_request.body.contentLen = 0;
	_request.body.chunked = false;
	_request.body.nextChunkSize = 0;
	_request.body.totalChunked = 0;
	_request.body.formDelimiter.clear();
	_request.bufferVec.clear();
	_request.body.contentVec.clear();
	//	response
	_response.headers.clear();
	resetStringStream(_response.body);
	_response.keepalive = true;
	_response.readFile = false;
	_response.cgi = false;
	_response.readFd = -1;
	// cgi
	_cgi.env.clear();
	_cgi.scriptname.clear();
	_cgi.pid = -1;
	_cgi.state = cgiState::BEGIN;
	_cgi.write_offset = 0;
	_cgi.output = false;
	_cgi.input.clear();
}

void httpHandler::httpClearCgi(void)
{
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
	resetStringStream(_response.body);
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
void httpHandler::addStringBuffer(char *buffer, size_t bytesRead)
{
	if (!_request.headCompleted)
	{
		// Insert new data into the buffer vector
		_request.bufferVec.insert(_request.bufferVec.end(), buffer, buffer + bytesRead);

		// Define the pattern to search for the end of the HTTP header
		static const char headerEndPattern[] = "\r\n\r\n";
		auto headEnd = std::search(
			_request.bufferVec.begin(),
			_request.bufferVec.end(),
			std::begin(headerEndPattern),
			std::end(headerEndPattern) - 1);

		if (headEnd != _request.bufferVec.end())
		{
			_request.headCompleted = true;
			std::string tempHead(_request.bufferVec.begin(), headEnd);
			_request.head << tempHead;
			parseHead();
			if (_statusCode > eHttpStatusCode::Accepted)
			{
				_request.keepReading = false;
				return;
			}
			setContent();
			if (_statusCode > eHttpStatusCode::Accepted || _request.body.contentType == eContentType::error || _request.body.contentType == eContentType::noContent)
			{
				_request.keepReading = false;
				return;
			}

			// Check if there is remaining data after the header
			if (_request.bufferVec.size() > tempHead.size() + 4)
			{
				// Create a vector for the remaining data
				std::vector<char> tempVec(headEnd + 4, _request.bufferVec.end());
				_request.bufferVec.clear();
				addToBody(tempVec.data(), tempVec.size());
			}
		}
		return;
	}
	else
	{
		addToBody(buffer, bytesRead);
	}
}

/**
 * @brief returns either READ_BUFFER_SIZE or the size of next chunk
 */
size_t httpHandler::getReadSize(void) const
{
	if (_request.body.chunked == true && _request.body.nextChunkSize != 0)
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
	for (char *envVar : env)
	{
		free(envVar);
	}
	env.clear();
	scriptname.clear();
	pid = -1;
	state = cgiState::BEGIN;
	write_offset = 0;
	output = false;
	htmlOutput = false;
	input.clear();
	output.clear();
	closeAllPipes();
}

void s_cgi::closeAllPipes(void)
{
	protectedClose(cgiIN[0]);
	protectedClose(cgiIN[1]);
	protectedClose(cgiOUT[0]);
	protectedClose(cgiOUT[1]);
}

s_cgi httpHandler::getCGI()
{
	_cgi.input = _request.body.content.str();
	return this->_cgi;
}

void resetStringStream(std::stringstream &ss)
{
	ss.str("");	 // Clear the content
	ss.clear();	 // Clear any error flags
	ss.seekg(0); // Reset the position to the beginning
	ss.seekp(0); // Reset the position to the beginning
}

void s_httpSend::clearHttpSend(void)
{
	msg.clear();
	keepAlive = true;
	readfile = false;
	readFd = -1;
	cgi = false;
}

/* debugging */

void httpHandler::logClassData(void) const
{
	std::cout << "\n------------ httpHandler Data Log ------------" << std::endl;
	std::cout << "Status Code: " << static_cast<int>(_statusCode) << std::endl;

	// Request data
	std::cout << "Request Method: " << static_cast<int>(_request.method) << std::endl;
	std::cout << "Request URI: " << _request.uri << std::endl;
	std::cout << "Request Path: " << _request.path << std::endl;
	std::cout << "Request Headers: " << std::endl;
	std::cout << "Number of Request Headers: " << _request.headers.size() << std::endl;
	std::cout << "Request Body Length: " << _request.body.contentLen << std::endl;
	std::cout << "Request Body Content Size: " << _request.body.content.str().size() << std::endl; // Added line
	std::cout << "Request Body Content: " << _request.body.content.str() << std::endl;

	// Response data
	std::cout << "Number of Response Headers: " << _response.headers.size() << std::endl;
	std::cout << "Response Body Size: " << _response.body.str().size() << std::endl; // Added line
	std::cout << "Response Body: " << _response.body.str() << std::endl;

	// CGI data
	std::cout << "CGI State: " << static_cast<int>(_cgi.state) << std::endl;
	std::cout << "CGI Input Size: " << _cgi.input.size() << std::endl; // Added line
	std::cout << "CGI Input: " << _cgi.input << std::endl;
	std::cout << "CGI Output Size: " << _cgi.output.size() << std::endl; // Added line
	std::cout << "CGI Output: " << _cgi.output << std::endl;
}