/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 13:01:04 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include "../../include/web.hpp"

/* constructor and deconstructor */

httpHandler::httpHandler(Server &server) : _server(server)
{
}

httpHandler::~httpHandler(void)
{
}

// default cgi request to unpack
std::string cgi = "POST /cgi-bin/script.cgi HTTP/1.1\r\n"
				  "Host: www.example.com\r\n"
				  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
				  "Content-Type: application/x-www-form-urlencoded\r\n"
				  "Content-Length: 27\r\n"
				  "\r\n"
				  "name=John&age=30\r\n";

// default POST request to unpack
std::string post = "POST /submit-form HTTP/1.1\r\n"
				   "Host: www.example.com\r\n"
				   "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
				   "Content-Type: application/x-www-form-urlencoded\r\n"
				   "Content-Length: 27\r\n"
				   "\r\n"
				   "name=John&age=30\r\n";

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

const std::unordered_map<eHttpStatusCode, std::string> statusMessages = {
	{eHttpStatusCode::NotSet, "Bad Request"},
	{eHttpStatusCode::Continue, "Continue"},
	{eHttpStatusCode::SwitchingProtocols, "Switching Protocols"},
	{eHttpStatusCode::OK, "OK"},
	{eHttpStatusCode::Created, "Created"},
	{eHttpStatusCode::Accepted, "Accepted"},
	{eHttpStatusCode::NonAuthoritativeInformation, "Non-Authoritative Information"},
	{eHttpStatusCode::NoContent, "No Content"},
	{eHttpStatusCode::ResetContent, "Reset Content"},
	{eHttpStatusCode::PartialContent, "Partial Content"},
	{eHttpStatusCode::MultipleChoices, "Multiple Choices"},
	{eHttpStatusCode::MovedPermanently, "Moved Permanently"},
	{eHttpStatusCode::Found, "Found"},
	{eHttpStatusCode::SeeOther, "See Other"},
	{eHttpStatusCode::NotModified, "Not Modified"},
	{eHttpStatusCode::UseProxy, "Use Proxy"},
	{eHttpStatusCode::TemporaryRedirect, "Temporary Redirect"},
	{eHttpStatusCode::PermanentRedirect, "Permanent Redirect"},
	{eHttpStatusCode::BadRequest, "Bad Request"},
	{eHttpStatusCode::Unauthorized, "Unauthorized"},
	{eHttpStatusCode::PaymentRequired, "Payment Required"},
	{eHttpStatusCode::Forbidden, "Forbidden"},
	{eHttpStatusCode::NotFound, "Not Found"},
	{eHttpStatusCode::MethodNotAllowed, "Method Not Allowed"},
	{eHttpStatusCode::NotAcceptable, "Not Acceptable"},
	{eHttpStatusCode::ProxyAuthenticationRequired, "Proxy Authentication Required"},
	{eHttpStatusCode::RequestTimeout, "Request Timeout"},
	{eHttpStatusCode::Conflict, "Conflict"},
	{eHttpStatusCode::Gone, "Gone"},
	{eHttpStatusCode::LengthRequired, "Length Required"},
	{eHttpStatusCode::PreconditionFailed, "Precondition Failed"},
	{eHttpStatusCode::PayloadTooLarge, "Payload Too Large"},
	{eHttpStatusCode::URITooLong, "URI Too Long"},
	{eHttpStatusCode::UnsupportedMediaType, "Unsupported Media Type"},
	{eHttpStatusCode::RangeNotSatisfiable, "Range Not Satisfiable"},
	{eHttpStatusCode::ExpectationFailed, "Expectation Failed"},
	{eHttpStatusCode::IAmATeapot, "I'm a teapot"},
	{eHttpStatusCode::MisdirectedRequest, "Misdirected Request"},
	{eHttpStatusCode::UnprocessableEntity, "Unprocessable Entity"},
	{eHttpStatusCode::Locked, "Locked"},
	{eHttpStatusCode::FailedDependency, "Failed Dependency"},
	{eHttpStatusCode::UpgradeRequired, "Upgrade Required"},
	{eHttpStatusCode::PreconditionRequired, "Precondition Required"},
	{eHttpStatusCode::TooManyRequests, "Too Many Requests"},
	{eHttpStatusCode::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large"},
	{eHttpStatusCode::UnavailableForLegalReasons, "Unavailable For Legal Reasons"},
	{eHttpStatusCode::InternalServerError, "Internal Server Error"},
	{eHttpStatusCode::NotImplemented, "Not Implemented"},
	{eHttpStatusCode::BadGateway, "Bad Gateway"},
	{eHttpStatusCode::ServiceUnavailable, "Service Unavailable"},
	{eHttpStatusCode::GatewayTimeout, "Gateway Timeout"},
	{eHttpStatusCode::HTTPVersionNotSupported, "HTTP Version Not Supported"},
	{eHttpStatusCode::VariantAlsoNegotiates, "Variant Also Negotiates"},
	{eHttpStatusCode::InsufficientStorage, "Insufficient Storage"},
	{eHttpStatusCode::LoopDetected, "Loop Detected"},
	{eHttpStatusCode::NotExtended, "Not Extended"},
	{eHttpStatusCode::NetworkAuthenticationRequired, "Network Authentication Required"}};

/* member functions */

s_location findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks)
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
std::optional<std::string> findHeaderValue(const s_request &request, eRequestHeader headerKey)
{
	for (const auto &header : request.headers)
	{
		if (header.first == headerKey)
		{
			return header.second;
		}
	}
	return std::nullopt;
}

std::string httpHandler::parseResponse(const std::string &httpRequest)
{
	std::istringstream ss(httpRequest);
	std::string requestLine;

	// what this function generates:

	// Get the request line
	if (!std::getline(ss, requestLine))
	{
		std::cerr << "Failed to read request line" << std::endl;
		return generateHttpResponse(eHttpStatusCode::BadRequest);
	}
	/* handle request line -> to be turned into seperate function*/
	std::istringstream requestss(requestLine);
	std::string methodstring, version;
	if (!(requestss >> methodstring >> _request.uri >> version))
		return (generateHttpResponse(eHttpStatusCode::BadRequest));

	// check version
	if (version != "HTTP/1.1")
		return (generateHttpResponse(eHttpStatusCode::HTTPVersionNotSupported));

	// check METHOD
	_request.method = _server.allowedHttpMethod(methodstring);
	if (_request.method == eHttpMethod::INVALID) // method check
		return (generateHttpResponse(eHttpStatusCode::MethodNotAllowed));

	// URI match against location
	_request.loc = findLongestPrefixMatch(_request.uri, _server.getLocation());
	if (_request.loc.cgi_ext == _request.uri.substr(_request.uri.find_last_of(".") + 1)) // cgi extension
	{
		cgi = true;
		if (!_request.loc.root.empty())
			_request.path = "." + _request.loc.root + _request.uri;
		else
			_request.path = "." + _server.getRoot() + _request.uri;
		if (!std::filesystem::exists(_request.path))
			return (generateHttpResponse(eHttpStatusCode::NotFound));
	}
	else
	{
		if (!_request.loc.root.empty())
			_request.path = "." + _request.loc.root + _request.uri;
		else
			_request.path = "." + _server.getRoot() + _request.uri;
		if (!std::filesystem::exists(_request.path))
			return (generateHttpResponse(eHttpStatusCode::NotFound));
	}

	// Read headers
	std::string header;
	std::string key, value;
	while (std::getline(ss, header) && !header.empty())
	{
		std::cout << "Header: " << header << std::endl;
		std::istringstream split(header);
		getline(split, key, ':');
		getline(split >> std::ws, value);
		eRequestHeader headerType = toEHeader(key);
		if (headerType == Invalid)
			return (generateHttpResponse(eHttpStatusCode::BadRequest));
		_request.headers.push_back(std::make_pair(headerType, value)); // or use {headerType, value}
	}
	std::optional<std::string> length = findHeaderValue(_request, eRequestHeader::ContentLength);
	if (length.has_value() && (std::stoi(length.value()) != 0))
	{
		// get body
		std::string body;
		while (std::getline(ss, body))
		{
			_request.body.append(body);
		}
	}

	std::string response;
	if (_request.cgi == true)
		cgiRequest();
	else
		stdRequest();
	return (response);
}

/* private functions */

eRequestHeader httpHandler::toEHeader(const std::string &header)
{
	static const std::unordered_map<std::string, eRequestHeader> headerMap = {
		{"Host", Host},
		{"User-Agent", UserAgent},
		{"Content-Type", ContentType},
		{"Content-Length", ContentLength}};
	auto it = headerMap.find(header);
	return it != headerMap.end() ? it->second : Invalid;
}

std::string httpHandler::generateHttpResponse(eHttpStatusCode statusCode)
{
	if (statusCode == eHttpStatusCode::NotSet)
		statusCode = eHttpStatusCode::BadRequest;
	std::string message;
	auto it = statusMessages.find(statusCode);
	if (it != statusMessages.end())
		message = it->second;
	else
	{
		message = "Bad request";
		statusCode = eHttpStatusCode::BadRequest;
	}
	std::ostringstream response;
	response << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << message << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << message.size() << "\r\n"
			 << "Connection: close\r\n"
			 << "\r\n"
			 << message;

	return response.str();
}
