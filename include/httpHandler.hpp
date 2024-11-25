/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 12:33:45 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/25 16:08:20 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"
#include "httpConstants.hpp"

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
	eHttpMethod method;
	std::string uri;
	s_location loc;
	std::string path;
	std::unordered_map<eRequestHeader, std::string> headers;
	std::stringstream body;
	bool cgi;
};

struct s_response
{
	std::unordered_map<eResponseHeader, std::string> headers;
	std::stringstream body;
};

struct s_httpSend;

class httpHandler
{
private:
	Server &_server;
	eHttpStatusCode _statusCode;
	s_request _request;
	s_response _response;

	// headers to strings and back
	eRequestHeader toEHeader(const std::string &header);
	std::string EheaderToString(const eRequestHeader &header);
	std::string responseHeaderToString(const eResponseHeader &header);
	std::optional<std::string> findHeaderValue(const s_request &request, eRequestHeader headerKey);
	// utils
	std::optional<s_location> findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks);
	std::optional<std::string> readFile(std::string &filename);
	std::string contentType(const std::string &filePath);
	// parse
	void parseRequestLine(std::stringstream &ss);
	void parseHeaders(std::stringstream &ss);
	void parseBody(std::stringstream &ss);
	// parse body
	void parseChunkedBody(std::stringstream &ss, const std::optional<std::string> &contentType);
	void parseFixedLengthBody(std::stringstream &ss, size_t length);
	void decodeContentEncoding(std::stringstream &body, const std::string &encoding);
	void parseMultipartBody(std::istream &ss, const std::string &contentType);
	std::string extractBoundary(const std::string &contentType);
	std::string extractHeaderValue(const std::string &headers, const std::string &key);
	std::string extractFilename(const std::string &contentDisposition);
	void saveFile(const std::string &filename, const std::string &fileData);
	// response
	s_httpSend writeResponse(bool keepalive);
	void generateDirectoryListing(void);
	// std Response
	void stdResponse(void);
	void stdGet(void);
	void stdPost(void);
	void stdDelete(void);
	// cgi Response
	void cgiResponse(void);

public:
	/* constructor and deconstructor */
	httpHandler(Server &server);
	~httpHandler(void);

	/* member functions */
	void parseRequest(std::stringstream &response);
	s_httpSend generateResponse(void);
};

#endif /* HTTP_HANDLER_HPP */
