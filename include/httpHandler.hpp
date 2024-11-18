/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 14:31:03 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 15:17:32 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"

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
	eHttpStatusCode statusCode = eHttpStatusCode::NotSet;
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

	// parse
	eRequestHeader toEHeader(const std::string &header);
	std::optional<std::string> findHeaderValue(const s_request &request, eRequestHeader headerKey);
	bool parseRequestLine(std::istringstream &ss);
	bool parseHeaders(std::istringstream &ss);

	// response
	std::string generateHttpResponse(eHttpStatusCode statusCode);
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
