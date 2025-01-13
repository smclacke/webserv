/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   httpHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 12:33:45 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/09 16:23:20 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "web.hpp"
#include "server.hpp"
#include "httpConstants.hpp"
#include "epoll.hpp"

struct s_location;

/**
 * @brief defines how the body should be read
 * @param error = error in headers, dont read body
 * @param ContentLength = Read until ContentLength is reached
 * @param formEncoded = Find delimiter and Read until delim is reached
 */
enum class eContentType
{
	error,
	noContent,
	formData,
	application,
	contentLength,
	undefined
};

struct s_content
{
	std::stringstream content;
	eContentType contentType;
	size_t contentLen;
	bool chunked;
	size_t nextChunkSize;
	size_t totalChunked;
	std::string formDelimiter;
};

/**
 * @param statusCode = statusCode to be responded
 * @param method = method called in HTTPSrequest
 * @param uri = uri
 * @param loc = the s_location from the Server relevant to this request
 * @param path = combination of the uri and the location root
 * @param headers = all headers in the HTTPsrequest
 * @param cgiReq = true if cgi request, false if its not a cgi request
 */
struct s_request
{
	bool keepReading;
	eHttpMethod method;
	std::string uri;
	s_location loc;
	bool cgiReq;
	std::string path;
	std::unordered_map<eRequestHeader, std::string> headers;
	bool uriEncoded;
	std::string uriQuery;
	std::stringstream head;
	bool headCompleted;
	s_content body;
};

struct s_response
{
	std::unordered_map<eResponseHeader, std::string> headers;
	std::stringstream body;
	bool keepalive = true;
	bool readFile = false;
	int readFd = -1;
	bool cgi = false;
};

struct s_httpSend;
class Epoll;

class httpHandler
{
private:
	Server &_server;
	Epoll &_epoll;
	eHttpStatusCode _statusCode;
	s_request _request;
	s_response _response;
	s_cgi _cgi;

	// headers to strings and back
	eRequestHeader toEHeader(const std::string &header);
	std::string EheaderToString(const eRequestHeader &header);
	std::string responseHeaderToString(const eResponseHeader &header);
	std::optional<std::string> findHeaderValue(const s_request &request, eRequestHeader headerKey);
	// utils
	std::optional<s_location> findLongestPrefixMatch(const std::string &requestUri, const std::vector<s_location> &locationBlocks);
	std::string contentType(const std::string &filePath);
	void setErrorResponse(eHttpStatusCode code, std::string msg);
	void CallErrorPage(std::string &path);
	bool generateEnv(void);
	// parse request+ headers
	void parseHead(void);
	void parseRequestLine(void);
	bool checkMethod(void);
	bool checkRedirect(void);
	void checkPath(void);
	std::string buildCgiPath(void);
	std::string buildPath(void);
	void parseHeaders(void);
	void setContent(void);
	// parse body
	void addToBody(std::string &buffer);
	void parseChunkedBody(std::string &buffer);
	void parseFixedLengthBody(std::string &buffer);
	void parseformData(std::string &buffer);
	// response
	s_httpSend writeResponse(void);
	void generateDirectoryListing(void);
	/* Response */
	void callMethod(void);
	// GET METHOD
	void getMethod(void);
	void getUriEncoded(void);
	bool getDirectory(void);
	void readFile(void);
	bool isContentTypeAccepted(const std::string &filePath, const std::string &acceptHeader);
	void openFile(void);
	// POST METHOD
	void postMethod(void);
	void generateEmptyFile(void);
	void postMultiForm(void);
	void parseMultipartBody(std::list<std::string> &files);
	std::string extractHeaderValue(const std::string &headers, const std::string &key);
	std::string extractFilename(const std::string &contentDisposition);
	std::string getTempFilePath(const std::string &filename);
	bool checkExecRights(void);
	void postUrlEncoded(void);
	void postApplication(void);
	void plainText(void);
	// DELETE METHOD
	void stdDelete(void);
	// cgi Response
	void cgiResponse();

public:
	/* constructor and deconstructor */
	httpHandler(Server &server, Epoll &epoll);
	~httpHandler(void);

	/* member functions */
	void clearHandler(void);
	void httpClearCgi(void);
	void addStringBuffer(std::string buffer);
	s_httpSend generateResponse(void);

	/* for epoll read operations */
	s_cgi getCGI();
	bool getKeepReading(void) const;
	size_t getReadSize(void) const;
};

std::string httpMethodToStringFunc(eHttpMethod method);
void resetStringStream(std::stringstream &ss);
std::string internalError(std::string msg);

#endif /* HTTP_HANDLER_HPP */
