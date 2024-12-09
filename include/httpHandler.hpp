/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 12:33:45 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/09 20:54:30 by jde-baai      ########   odam.nl         */
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
	std::vector<std::string> files;
	bool uriEncoded;
};

struct s_response
{
	std::unordered_map<eResponseHeader, std::string> headers;
	std::stringstream body;
	bool keepalive = true;
	bool readFile = false;
	bool cgi = false;
	int readFd = -1;
	pid_t pid = -1;
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
	void openFile(std::string &path);
	std::string contentType(const std::string &filePath);
	void setErrorResponse(eHttpStatusCode code, std::string msg);
	std::string buildPath(void);
	void CallErrorPage(std::string &path);
	// parse request+ headers
	void parseRequestLine(std::stringstream &ss);
	bool checkRedirect();
	void parseHeaders(std::stringstream &ss);
	void parseBody(std::stringstream &ss);
	void checkPath(void);
	// parse body
	void parseChunkedBody(std::stringstream &ss);
	void parseFixedLengthBody(std::stringstream &ss, size_t length);
	void decodeContentEncoding(std::stringstream &body, const std::string &encoding);
	void parseMultipartBody(const std::string &contentType);
	std::string extractBoundary(const std::string &contentType);
	std::string extractHeaderValue(const std::string &headers, const std::string &key);
	std::string extractFilename(const std::string &contentDisposition);
	std::string getTempFilePath(const std::string &filename);
	// response
	s_httpSend writeResponse(void);
	void generateDirectoryListing(void);
	// std Response
	void callMethod(void);
	void stdGet(void);
	void getUriEncoded(void);
	void readFile();
	void stdPost(void);
	void wwwFormEncoded(void);
	void stdDelete(void);
	void deleteFromCSV();
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
