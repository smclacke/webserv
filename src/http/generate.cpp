/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generate.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:52:04 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/05 16:13:05 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/error.hpp"

enum class HttpStatusCode
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

const std::string HTTP_VERSION = "HTTP/1.1";
const std::string CONNECTION_CLOSE = "Connection: close";			// Default connection close
const std::string CONNECTION_KEEP_ALIVE = "Connection: keep-alive"; // Alternative for keep-alive

/* things about origin form and request target */

// Define common content types
const std::string CONTENT_TYPE_TEXT = "Content-Type: text/plain";
// const std::string CONTENT_TYPE_HTML = "Content-Type: text/html";
// const std::string CONTENT_TYPE_JSON = "Content-Type: application/json";
// const std::string CONTENT_TYPE_XML = "Content-Type: application/xml";
// const std::string CONTENT_TYPE_JAVASCRIPT = "Content-Type: application/javascript";
// const std::string CONTENT_TYPE_CSS = "Content-Type: text/css";
// const std::string CONTENT_TYPE_OCTET_STREAM = "Content-Type: application/octet-stream";

std::string generateHttpResponse(const std::string &message)
{
	size_t contentLength = message.size();
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << contentLength << "\r\n"
			 << "Connection: close\r\n"
			 << "\r\n"
			 << message;

	return response.str();
}

/**
The request line in an HTTP request message contains the method, URI, and HTTP version. Parsing this line is the first step in understanding the client’s request.
For example:

GET /path/to/resource HTTP/1.1
 */

/**
 linebreak:

#define LINEBREAK "\r\n"
linebreak in http = CRLF = \r\n



		  +-------+-----------------------------+---------------+
		  | Index | Header Name                 | Header Value  |
		  +-------+-----------------------------+---------------+
		  | 1     | :authority                  |               |
		  | 2     | :method                     | GET           |
		  | 3     | :method                     | POST          |
		  | 4     | :path                       | /             |
		  | 5     | :path                       | /index.html   |
		  | 6     | :scheme                     | http          |
		  | 7     | :scheme                     | https         |
		  | 8     | :status                     | 200           |
		  | 9     | :status                     | 204           |
		  | 10    | :status                     | 206           |
		  | 11    | :status                     | 304           |
		  | 12    | :status                     | 400           |
		  | 13    | :status                     | 404           |
		  | 14    | :status                     | 500           |
		  | 15    | accept-charset              |               |
		  | 16    | accept-encoding             | gzip, deflate |
		  | 17    | accept-language             |               |
		  | 18    | accept-ranges               |               |
		  | 19    | accept                      |               |
		  | 20    | access-control-allow-origin |               |
		  | 21    | age                         |               |
		  | 22    | allow                       |               |
		  | 23    | authorization               |               |
		  | 24    | cache-control               |               |
		  | 25    | content-disposition         |               |
		  | 26    | content-encoding            |               |
		  | 27    | content-language            |               |
		  | 28    | content-length              |               |
		  | 29    | content-location            |               |
		  | 30    | content-range               |               |
		  | 31    | content-type                |               |
		  | 32    | cookie                      |               |
		  | 33    | date                        |               |
		  | 34    | etag                        |               |
		  | 35    | expect                      |               |
		  | 36    | expires                     |               |
		  | 37    | from                        |               |
		  | 38    | host                        |               |
		  | 39    | if-match                    |               |
		  | 40    | if-modified-since           |               |
		  | 41    | if-none-match               |               |
		  | 42    | if-range                    |               |
		  | 43    | if-unmodified-since         |               |
		  | 44    | last-modified               |               |
		  | 45    | link                        |               |
		  | 46    | location                    |               |
		  | 47    | max-forwards                |               |
		  | 48    | proxy-authenticate          |               |
		  | 49    | proxy-authorization         |               |
		  | 50    | range                       |               |
		  | 51    | referer                     |               |
		  | 52    | refresh                     |               |
		  | 53    | retry-after                 |               |
		  | 54    | server                      |               |
		  | 55    | set-cookie                  |               |
		  | 56    | strict-transport-security   |               |
		  | 57    | transfer-encoding           |               |
		  | 58    | user-agent                  |               |
		  | 59    | vary                        |               |
		  | 60    | via                         |               |
		  | 61    | www-authenticate            |               |
		  +-------+-----------------------------+---------------+

					   Table 1: Static Table Entries


 */

/**
 * CGI
 *

A Web server that supports CGI can be configured to interpret a URL that it serves as a reference
to a CGI script. A common convention is to have a cgi-bin/ directory at the base of the directory
tree and treat all executable files within this directory (and no other, for security) as CGI scripts.
When a Web browser requests a URL that points to a file within the CGI directory
(e.g., http://example.com/cgi-bin/printenv.pl/with/additional/path?and=a&query=string),
then, instead of simply sending that file (/usr/local/apache/htdocs/cgi-bin/printenv.pl) to the Web browser,
the HTTP server runs the specified script and passes the output of the script to the Web browser.
That is, anything that the script sends to standard output is passed to the Web client instead
of being shown in the terminal window that started the web server.

Another popular convention is to use filename extensions; for instance,
if CGI scripts are consistently given the extension .cgi,
the Web server can be configured to interpret all such files as CGI scripts.
While convenient, and required by many prepackaged scripts,
it opens the server to attack if a remote user can upload executable code with the proper extension.
The CGI specification defines how additional information passed with the request is passed to the script.
The Web server creates a subset of the environment variables passed to it and
adds details pertinent to the HTTP environment.
For instance, if a slash and additional directory name(s) are appended to the URL immediately
after the name of the script (in this example, /with/additional/path),
then that path is stored in the PATH_INFO environment variable before the script is called.
If parameters are sent to the script via an HTTP GET request (a question mark appended to the URL,
followed by param=value pairs; in the example, ?and=a&query=string),
then those parameters are stored in the QUERY_STRING environment variable before the script is called.
Request HTTP message body, such as form parameters sent via an HTTP POST request,
are passed to the script's standard input.
The script can then read these environment variables or data from standard input and
adapt to the Web browser's request.[8]


Do you wonder what a CGI is?
∗ Because you won’t call the CGI directly, use the full path as PATH_INFO.
∗ Just remember that, for chunked request, your server needs to unchunk
it, the CGI will expect EOF as end of the body.
∗ Same things for the output of the CGI. If no content_length is returned
from the CGI, EOF will mark the end of the returned data.
∗ Your program should call the CGI with the file requested as first argument.
∗ The CGI should be run in the correct directory for relative path file access.
∗ Your server should work with one CGI (php-CGI, Python, and so forth).


 */