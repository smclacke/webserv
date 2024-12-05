/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpConstants.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: juliusdebaaij <juliusdebaaij@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/24 11:43:35 by juliusdebaa   #+#    #+#                 */
/*   Updated: 2024/12/05 15:10:02 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONSTANTS_HPP
#define HTTP_CONSTANTS_HPP

#include <string>
#include <unordered_map>

enum class eHttpStatusCode
{
	NotSet = 0,
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

enum class eRequestHeader
{
	Host,			  // Server domain and optional port.
	UserAgent,		  // Info about the requesting user agent.
	ContentType,	  // Media type of the request body.
	ContentLength,	  // Size of the request body in bytes.
	TransferEncoding, // Encoding used for the payload.
	ContentEncoding,  // Compression encoding for the body.
	Accept,			  // Acceptable media types for response.
	Connection,		  // Network connection persistence.
	// from here headers not used / implemented
	Authorization,				 // Credentials for user authentication.
	CacheControl,				 // Caching directives.
	Cookie,						 // Stored HTTP cookies.
	Date,						 // Message origination date and time.
	Expect,						 // Required server behaviors.
	Origin,						 // CORS request origin.
	Referer,					 // Previous page address.
	ContentDisposition,			 // Handling of response payload.
	IfModifiedSince,			 // Conditional request based on modification date.
	IfNoneMatch,				 // Conditional request based on ETag.
	IfMatch,					 // Conditional request based on matching ETag.
	IfUnmodifiedSince,			 // Conditional request based on unmodified date.
	AcceptCharset,				 // Acceptable character sets.
	AcceptEncoding,				 // Acceptable content encodings.
	AcceptLanguage,				 // Preferred response languages.
	AccessControlRequestMethod,	 // Preflight request method.
	AccessControlRequestHeaders, // Preflight request headers.
	Forwarded,					 // Client and proxy information.
	From,						 // Requesting user agent's email.
	MaxForwards,				 // Limit on request forwarding.
	ProxyAuthorization,			 // Proxy server authentication credentials.
	Range,						 // Request part of an entity.
	TE,							 // Acceptable transfer encodings.
	UpgradeInsecureRequests,	 // Preference for secure response.
	Via,						 // Proxy information.
	Warning,					 // Additional message status info.
	Invalid						 // Unrecognized or malformed headers.
};

const std::unordered_map<std::string, eRequestHeader> headerMap = {
	{"Host", eRequestHeader::Host},
	{"User-Agent", eRequestHeader::UserAgent},
	{"Content-Type", eRequestHeader::ContentType},
	{"Content-Length", eRequestHeader::ContentLength},
	{"Transfer-Encoding", eRequestHeader::TransferEncoding},
	{"Content-Encoding", eRequestHeader::ContentEncoding},
	{"Accept", eRequestHeader::Accept},
	{"Authorization", eRequestHeader::Authorization},
	{"Cache-Control", eRequestHeader::CacheControl},
	{"Connection", eRequestHeader::Connection},
	{"Cookie", eRequestHeader::Cookie},
	{"Date", eRequestHeader::Date},
	{"Expect", eRequestHeader::Expect},
	{"Origin", eRequestHeader::Origin},
	{"Referer", eRequestHeader::Referer},
	{"Content-Disposition", eRequestHeader::ContentDisposition},
	{"If-Modified-Since", eRequestHeader::IfModifiedSince},
	{"If-None-Match", eRequestHeader::IfNoneMatch},
	{"If-Match", eRequestHeader::IfMatch},
	{"If-Unmodified-Since", eRequestHeader::IfUnmodifiedSince},
	{"Accept-Charset", eRequestHeader::AcceptCharset},
	{"Accept-Encoding", eRequestHeader::AcceptEncoding},
	{"Accept-Language", eRequestHeader::AcceptLanguage},
	{"Access-Control-Request-Method", eRequestHeader::AccessControlRequestMethod},
	{"Access-Control-Request-Headers", eRequestHeader::AccessControlRequestHeaders},
	{"Forwarded", eRequestHeader::Forwarded},
	{"From", eRequestHeader::From},
	{"Max-Forwards", eRequestHeader::MaxForwards},
	{"Proxy-Authorization", eRequestHeader::ProxyAuthorization},
	{"Range", eRequestHeader::Range},
	{"TE", eRequestHeader::TE},
	{"Upgrade-Insecure-Requests", eRequestHeader::UpgradeInsecureRequests},
	{"Via", eRequestHeader::Via},
	{"Warning", eRequestHeader::Warning}};

/**
 *@note add connection Responseheader, check when its supposed to be set;
 */
enum class eResponseHeader
{
	ContentType,	 // the media type of the resource.
	ContentLength,	 // The size of the response body in bytes.
	ContentEncoding, // Compression encoding for the body.
	Connection,
	// implemented till here
	SetCookie,				  // Used to send cookies from the server to the client.
	CacheControl,			  // for caching mechanisms in both requests and responses.
	Expires,				  // Provides a date/time after which the response is considered stale.
	ETag,					  // A unique identifier for a specific version of a resource.
	LastModified,			  // The date and time at which the resource was last modified.
	Location,				  // Used in redirection or when a new resource has been created.
	WWWAuthenticate,		  // the authentication scheme for accessing the resource.
	RetryAfter,				  // how long to wait before making a follow-up request.
	AccessControlAllowOrigin, // which origins are allowed to access the resource (CORS).
	StrictTransportSecurity,  // secure connections to the server.
	Vary,					  // which headers affect the response's cacheability.
	Server,					  // information about the server software.
	ContentDisposition		  // Suggests a default filename and disposition for the response payload.
};

#endif /* HTTP_CONSTANTS_HPP */