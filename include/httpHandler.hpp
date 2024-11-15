/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 14:31:03 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/15 18:03:10 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"

enum class eHttpStatusCode
{
	Continue = 100,
	SwitchingProtocols = 101,
	OK = 200,
	Created = 201,
	Accepted = 202,
	NonAuthoritativeInformation = 203,
	NoContent = 204,
	ResetContent = 205,
	PartialContent = 206,
	MultipleChoices = 300,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	UseProxy = 305,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,
	BadRequest = 400,
	Unauthorized = 401,
	PaymentRequired = 402,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	NotAcceptable = 406,
	ProxyAuthenticationRequired = 407,
	RequestTimeout = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PreconditionFailed = 412,
	PayloadTooLarge = 413,
	URITooLong = 414,
	UnsupportedMediaType = 415,
	RangeNotSatisfiable = 416,
	ExpectationFailed = 417,
	IAmATeapot = 418,
	MisdirectedRequest = 421,
	UnprocessableEntity = 422,
	Locked = 423,
	FailedDependency = 424,
	UpgradeRequired = 426,
	PreconditionRequired = 428,
	TooManyRequests = 429,
	RequestHeaderFieldsTooLarge = 431,
	UnavailableForLegalReasons = 451,
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504,
	HTTPVersionNotSupported = 505,
	VariantAlsoNegotiates = 506,
	InsufficientStorage = 507,
	LoopDetected = 508,
	NotExtended = 510,
	NetworkAuthenticationRequired = 511
};

enum eRequestHeader
{
	Host,
	UserAgent,
	ContentType,
	ContentLength,
	Invalid
};

enum class eHttpMethod;
struct s_location;

struct s_request
{
	eHttpMethod method;
	std::string uri;
	s_location loc;
	std::string path;
	std::vector<std::pair<eRequestHeader, std::string>> headers;
	std::string body;
	bool cgi = false;
};

class httpHandler
{
private:
	Server &_server;
	s_request _request;
	std::string generateHttpResponse(eHttpStatusCode statusCode);
	eRequestHeader toEHeader(const std::string &header);
	std::string cgiRequest(void);
	std::string stdRequest(void);

public:
	/* consturctor and deconstructor */
	httpHandler(Server &server);
	~httpHandler(void);

	/* member functions */
	std::string parseResponse(const std::string &response);
};

#endif /* HTTP_HANDLER_HPP */
