/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parse.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 17:15:45 by jde-baai      ########   odam.nl         */
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
void httpHandler::parseRequest(std::stringstream &httpRequest)
{
	if (httpRequest.str().empty())
	{
		std::cerr << "Received empty HTTP request" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Empty HTTP request");
	}

	std::cout << "\n======= REQUEST =======\n"
			  << httpRequest.str() << "\n======= END REQ =======\n";

	parseRequestLine(httpRequest);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	if (!checkRedirect())
		return;
	checkPath();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	parseHeaders(httpRequest);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	std::string remainingData;
	std::getline(httpRequest, remainingData, '\0');
	// Check if the remaining data is not just the end of headers
	if (!remainingData.empty() && remainingData != "\r\n\r\n")
	{
		// Place the remaining data back into the stream
		httpRequest.clear();			// Clear any error flags
		httpRequest.str(remainingData); // Reset the stream with the remaining data
		httpRequest.seekg(0);			// Reset the position to the beginning of the stream
		parseBody(httpRequest);
	}
	return;
}

/* private functions */

/**
 * @brief Parses the request line of the HTTP request
 */
void httpHandler::parseRequestLine(std::stringstream &ss)
{
	std::string requestLine;

	// Get the request line
	if (!std::getline(ss, requestLine))
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
void httpHandler::parseHeaders(std::stringstream &ss)
{
	std::string header;
	std::string key, value;
	while (std::getline(ss, header) && !header.empty() && header != "\r")
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
