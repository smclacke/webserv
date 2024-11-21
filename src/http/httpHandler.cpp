/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 17:21:12 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/21 13:55:45 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/* constructor and deconstructor */

httpHandler::httpHandler(Server &server) : _server(server)
{
	_request.statusCode = eHttpStatusCode::OK;
	_request.cgi = false;
}

httpHandler::~httpHandler(void)
{
}

/* utils */

s_location httpHandler::findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks)
{
	s_location longestMatch;

	for (const auto &location : locationBlocks)
	{
		// Check if the location is a prefix of the request URI
		if (requestUri.find(location.path) == 0)
		{ // Check if the requestUri starts with location
			if (location.path.length() > longestMatch.path.length())
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
