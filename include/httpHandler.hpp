/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 12:33:45 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/21 15:43:27 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"

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
	// from here headers not used / implemented
	Accept,						 // Acceptable media types for response.
	Authorization,				 // Credentials for user authentication.
	CacheControl,				 // Caching directives.
	Connection,					 // Network connection persistence.
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
 *
 */
enum class eResponseHeader
{
	ContentType,			  // the media type of the resource.
	ContentLength,			  // The size of the response body in bytes.
	ContentEncoding,		  // Compression encoding for the body.
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

enum class eHttpMethod;
struct s_location;

/**
 * @param statusCode = statusCode to be responded
 * @param method = method called in HTTPSrequest
 * @param uri = uri
 * @param loc = the s_location from the Server relevant to this request
 * @param path = combination of the uri and the location root
 * @param headers = all headers in the HTTPsrequest
 * @param cgi = true if cgi request, false if its not a cgi request
 */
struct s_request
{
	eHttpStatusCode statusCode;
	eHttpMethod method;
	std::string uri;
	s_location loc;
	std::string path;
	std::unordered_map<eRequestHeader, std::string> headers;
	std::stringstream body;
	bool cgi;
};
class httpHandler
{
private:
	Server &_server;
	s_request _request;

	// utils
	eRequestHeader toEHeader(const std::string &header);
	std::string EheaderToString(const eRequestHeader &header);
	std::optional<std::string> findHeaderValue(const s_request &request, eRequestHeader headerKey);
	s_location findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks);
	// parse
	void parseRequestLine(std::istringstream &ss);
	void parseHeaders(std::istringstream &ss);
	void parseBody(std::istringstream &ss);
	// parse body
	void parseChunkedBody(std::istringstream &ss, const std::optional<std::string> &contentType);
	void parseFixedLengthBody(std::istringstream &ss, size_t length);
	void decodeContentEncoding(std::stringstream &body, const std::string &encoding);
	void parseMultipartBody(std::istream &ss, const std::string &contentType);
	std::string extractBoundary(const std::string &contentType);
	std::string extractHeaderValue(const std::string &headers, const std::string &key);
	std::string extractFilename(const std::string &contentDisposition);
	void saveFile(const std::string &filename, const std::string &fileData);

	// response
	std::string
	writeResponse(void);
	std::string cgiRequest(void);
	std::string stdRequest(void);

public:
	/* constructor and deconstructor */
	httpHandler(Server &server);
	~httpHandler(void);

	/* member functions */
	void parseRequest(const std::string &response);
	std::string generateResponse();
};

#endif /* HTTP_HANDLER_HPP */
