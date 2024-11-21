/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/19 17:21:12 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/21 10:33:41 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/* constructor and deconstructor */

httpHandler::httpHandler(Server &server) : _server(server)
{
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

eRequestHeader toEHeader(const std::string &header)
{
	static const std::unordered_map<std::string, eRequestHeader> headerMap = {
		{"Host", Host},
		{"User-Agent", UserAgent},
		{"Content-Type", ContentType},
		{"Content-Length", ContentLength},
		{"Transfer-Encoding", TransferEncoding},
		{"Content-Encoding", ContentEncoding},
		{"Accept", Accept},
		{"Authorization", Authorization},
		{"Cache-Control", CacheControl},
		{"Connection", Connection},
		{"Cookie", Cookie},
		{"Date", Date},
		{"Expect", Expect},
		{"Origin", Origin},
		{"Referer", Referer},
		{"Content-Disposition", ContentDisposition},
		{"If-Modified-Since", IfModifiedSince},
		{"If-None-Match", IfNoneMatch},
		{"If-Match", IfMatch},
		{"If-Unmodified-Since", IfUnmodifiedSince},
		{"Accept-Charset", AcceptCharset},
		{"Accept-Encoding", AcceptEncoding},
		{"Accept-Language", AcceptLanguage},
		{"Access-Control-Request-Method", AccessControlRequestMethod},
		{"Access-Control-Request-Headers", AccessControlRequestHeaders},
		{"Forwarded", Forwarded},
		{"From", From},
		{"Max-Forwards", MaxForwards},
		{"Proxy-Authorization", ProxyAuthorization},
		{"Range", Range},
		{"TE", TE},
		{"Upgrade-Insecure-Requests", UpgradeInsecureRequests},
		{"Via", Via},
		{"Warning", Warning}};
	auto it = headerMap.find(header);
	return it != headerMap.end() ? it->second : Invalid;
}