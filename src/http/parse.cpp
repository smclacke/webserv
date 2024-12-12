/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parse.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/12 13:32:47 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include "../../include/web.hpp"

/* member functions */

/**
 * @brief Parses the https request and stores it in the httpHandler::_request
 */
void httpHandler::parseHead(void)
{
	if (_request.head.str().empty())
	{
		std::cerr << "Received empty HTTP request" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Empty HTTP request");
	}

	std::cout << "\n======= REQUEST HEADERS =======\n"
			  << _request.head.str() << "\n======= END REQ HEADERS =======\n";

	parseRequestLine();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	if (!checkRedirect())
		return;
	checkPath();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	parseHeaders();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	return;
}

/* private functions */

/**
 * @brief Parses the request line of the HTTP request
 */
void httpHandler::parseRequestLine(void)
{
	std::string requestLine;

	// Get the request line
	if (!std::getline(_request.head, requestLine))
	{
		std::cerr << "Failed to read request line" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Failed to read request line");
	}
	/* parse request line */
	std::istringstream requestss(requestLine);
	std::string methodstring, version;
	if (!(requestss >> methodstring >> _request.uri >> version))
	{
		std::cerr << "Invalid request line format" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Invalid request line format");
	}
	// check METHOD
	_request.method = _server.allowedHttpMethod(methodstring);
	if (_request.method == eHttpMethod::INVALID) // method check
	{
		std::cerr << "HTTP method not allowed: " << methodstring << std::endl;
		return setErrorResponse(eHttpStatusCode::MethodNotAllowed, "HTTP method not allowed: " + methodstring);
	}
	// Check if URI is too long
	const size_t MAX_URI_LENGTH = 2048; // Example limit, adjust as needed
	if (_request.uri.length() > MAX_URI_LENGTH)
	{
		std::cerr << "URI too long: " << _request.uri << std::endl;
		return setErrorResponse(eHttpStatusCode::URITooLong, "URI too long: " + _request.uri);
	}
	// check version
	if (version != "HTTP/1.1")
	{
		std::cerr << "HTTP version not supported: " << version << std::endl;
		return setErrorResponse(eHttpStatusCode::HTTPVersionNotSupported, "HTTP version not supported: " + version);
	}

	// URI match against location
	std::optional<s_location> optLoc = findLongestPrefixMatch(_request.uri, _server.getLocation());
	if (!optLoc.has_value())
	{
		std::cerr << "No Matching location for URI: " << _request.uri << std::endl;
		return setErrorResponse(eHttpStatusCode::NotFound, "No Matching location for URI: " + _request.uri);
	}
	_request.loc = optLoc.value();
}

/**
 * @brief check if there is a redirect
 * sets the right _response headers for the redirect
 * @return true if no redirect, false if there is a redirect
 */
bool httpHandler::checkRedirect(void)
{
	if (_request.loc.redirect_status != 0)
	{
		switch (_request.loc.redirect_status)
		{
		case 301:
			_statusCode = eHttpStatusCode::MovedPermanently;
			break;
		case 302:
			_statusCode = eHttpStatusCode::Found;
			break;
		case 303:
			_statusCode = eHttpStatusCode::SeeOther;
			break;
		case 307:
			_statusCode = eHttpStatusCode::TemporaryRedirect;
			break;
		case 308:
			_statusCode = eHttpStatusCode::PermanentRedirect;
			break;
		default:
			setErrorResponse(eHttpStatusCode::InternalServerError, "redirect status code doesn't match any expected value");
			return false;
		}
		_response.headers[eResponseHeader::Location] = _request.loc.redir_url;
		_response.headers[eResponseHeader::ContentLength] = "0";
		return false;
	}
	return (true);
}

/**
 * @brief builds the path.
 * Replaces the part of the uri that tags the location with the root of said location
 */
std::string httpHandler::buildPath(void)
{
	std::string buildpath;
	std::string uri = _request.uri;
	std::string locpath;

	if (_request.loc.path == "/")
		locpath = "";
	else
		locpath = _request.loc.path;
	if (!_request.loc.root.empty())
	{
		if (locpath.size() <= uri.size())
		{
			uri.erase(0, locpath.length());
			uri = _request.loc.root + uri;
		}
	}
	else if (!_server.getRoot().empty())
	{
		if (locpath.size() <= uri.size())
		{
			uri.erase(0, locpath.length());
			uri = _server.getRoot() + uri;
		}
	}
	buildpath = "." + uri;
	return (buildpath);
}

/**
 * @brief checks if the is a x-www-form-urlencoded request and if the path is valid
 */
void httpHandler::checkPath(void)
{
	size_t queryPos = _request.uri.find("?");
	if (queryPos != std::string::npos)
	{
		std::string tempPath = buildPath();
		if (std::filesystem::exists(tempPath))
		{
			_request.path = tempPath;
			return; // ? was a coincidence, not denfing URI
		}
		std::string uri = _request.uri;
		_request.uriQuery = uri.substr(queryPos + 1);
		_request.path = uri.erase(queryPos);
		if (_request.uriQuery.empty())
		{
			return setErrorResponse(eHttpStatusCode::BadRequest, "Uri encoding is empty");
		}
		_request.uriEncoded = true;
	}
	_request.path = buildPath();
	if (!std::filesystem::exists(_request.path))
	{
		std::cerr << "Resource not found at path: " << _request.path << std::endl;
		return setErrorResponse(eHttpStatusCode::NotFound, "Resource not found at path: " + _request.path);
	}
}

/**
 * @brief Parses the headers of the HTTP request
 */
void httpHandler::parseHeaders(void)
{
	std::string header;
	std::string key, value;
	while (std::getline(_request.head, header) && !header.empty() && header != "\r")
	{
		if (header.back() == '\r')
			header.pop_back();
		std::istringstream split(header);
		getline(split, key, ':');
		getline(split >> std::ws, value);
		eRequestHeader headerType = toEHeader(key);
		if (headerType == eRequestHeader::Invalid)
			continue;
		if (headerType == eRequestHeader::Connection)
		{
			if (value != "keep-alive" && value != "close")
			{
				_response.readFile = false;
				_response.cgi = false;
				return setErrorResponse(eHttpStatusCode::NotImplemented, "Connection type not implemented: " + value);
			}
		}
		_request.headers[headerType] = value;
	}
}

void httpHandler::setContent(void)
{
	auto contentLengthHeader = findHeaderValue(_request, eRequestHeader::ContentLength);
	auto contentTypeHeader = findHeaderValue(_request, eRequestHeader::ContentType);
	auto transferEncodingHeader = findHeaderValue(_request, eRequestHeader::TransferEncoding);

	if (contentLengthHeader.has_value())
	{
		_request.body.contentLen = std::stoul(contentLengthHeader.value());
		if (_request.body.contentLen > _request.loc.client_body_buffer_size)
		{
			_request.keepReading = false;
			return setErrorResponse(eHttpStatusCode::InsufficientStorage, "Fixed length body size exceeds client body buffer size");
		}
		if (_request.body.contentLen == 0)
		{
			_request.keepReading = false;
			_request.body.contentType = eContentType::noContent;
			return;
		}
		if (contentTypeHeader.has_value() && contentTypeHeader.value().find("multipart/form-data") != std::string::npos)
		{
			size_t pos = contentTypeHeader.value().find("boundary=");
			if (pos == std::string::npos)
			{
				setErrorResponse(eHttpStatusCode::BadRequest, "multipart/form-data has no boundary");
				_request.keepReading = false;
				return;
			}
			else
			{
				_request.body.formDelimiter = contentTypeHeader.value().substr(pos + 9);
			}
			_request.body.contentType = eContentType::formData;
			return;
		}
		else if (contentTypeHeader.has_value() && contentTypeHeader.value().find("application/") != std::string::npos)
		{
			_request.body.contentType = eContentType::application;
			return;
		}
		else
		{
			_request.body.contentType = eContentType::contentLength;
			return;
		}
	}
	else if (transferEncodingHeader.has_value())
	{
		if (transferEncodingHeader.value() == "chunked")
		{
			_request.body.contentType = eContentType::chunked;
		}
		else
		{
			_request.body.contentType = eContentType::error;
			return setErrorResponse(eHttpStatusCode::NotImplemented, "Transfer-Encoding type not implemented: " + transferEncodingHeader.value());
		}
	}
	else
	{
		_request.body.contentType = eContentType::noContent;
	}
}