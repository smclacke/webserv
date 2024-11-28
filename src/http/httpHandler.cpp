/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 17:21:12 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 14:38:03 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/* constructor and deconstructor */

httpHandler::httpHandler(Server &server) : _server(server)
{
	_statusCode = eHttpStatusCode::OK;
	_request.method = eHttpMethod::INVALID;
	_request.uri = "";
	_request.path = "";
	_request.body.str() = "";
	_request.uriEncoded = false;
	_request.cgi = false;
}

httpHandler::~httpHandler(void)
{
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
	_response.body.str() = msg;
	_response.headers[eResponseHeader::ContentLength] = std::to_string(msg.size());
}
