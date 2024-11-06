/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/06 15:15:41 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include <unordered_map>

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

std::string httpHandler::parseResponse(const std::string &httpRequest)
{
	try
	{

		std::istringstream ss(httpRequest);
		std::string requestLine;
		const std::unordered_map<std::string, std::string> httpHeaders;

		// Get the request line
		std::getline(ss, requestLine);
		std::cout << "Request Line: " << requestLine << std::endl;

		std::istringstream requestss(requestLine);
		std::string methodstring, uri, version;
		if (!(requestss >> methodstring >> uri >> version))
		{
			throw httpError("HTTP requesst line unexpected format: " + requestLine, eHttpStatusCode::BadRequest);
		}

		// Parse the request line
		eHttpMethod method = _server.allowedHttpMethod(methodstring);
		if (method == eHttpMethod::INVALID)
			return (generateHttpResponse(eHttpStatusCode::MethodNotAllowed));
		std::string method, uri, version;
		std::istringstream requestLineStream(requestLine);

		std::cout << "Method: " << methodstring << std::endl;
		std::cout << "URI: " << uri << std::endl;
		std::cout << "Version: " << version << std::endl;

		// Read headers
		std::string header;
		while (std::getline(ss, header) && !header.empty())
		{

			std::cout << "Header: " << header << std::endl;
		}
	}
	catch (httpError &e)
	{
		std::cerr << "HTTP request error: " << e.what() << std::endl;
		return (generateHttpResponse(e.code()));
	}
}

std::string getStatusMessage(eHttpStatusCode statusCode)
{
	auto it = statusMessages.find(statusCode);
	if (it != statusMessages.end())
		return it->second;
	return "Unknown Status";
}

std::string httpHandler::generateHttpResponse(eHttpStatusCode statusCode)
{
	std::string message = getStatusMessage(statusCode);
	std::ostringstream response;
	response << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << message << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << message.size() << "\r\n"
			 << "Connection: close\r\n"
			 << "\r\n"
			 << message;

	return response.str();
}
