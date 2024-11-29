/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/29 17:17:26 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_HPP
#define WEB_HPP

// CPP includes
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <limits>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>	  // shared pointers header
#include <filesystem> // for directory checking
#include <optional>
#include <atomic>

// CPP containers
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <list>
#include <deque>
#include <utility>
#include <unordered_map>

// Network includes
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket functions
#include <sys/epoll.h>
#include <sys/ioctl.h> // for nonblocking stuff..
#include <arpa/inet.h>
#include <sys/select.h> // for select()

// C includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <csignal>


/* forward declarations*/
struct s_location;
class Server;
class Webserv;

/* enums, defines and errors */

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

enum class eSocket
{
	Client = 1,
	Server = 2
};

/* http */
std::string generateHttpResponse(const std::string &message);

/* parser */
void lineStrip(std::string &line);
void verifyInput(int ac, char **av);
void checkLocationPaths(s_location &loc, std::string const root, int const line_n);
void findLocationDirective(std::string &line, int &line_n, s_location &loc);
void findServerDirective(Server &serv, std::string &line, int line_n);
s_location parseLocation(std::ifstream &file, std::string &line, int &line_n, size_t maxbody);

/* server */

/* utils */
bool	protectedClose(int fd);

#endif /* WEB_HPP */
