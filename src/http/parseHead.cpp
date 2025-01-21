/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parseHead.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/05 14:48:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/21 14:48:59 by jde-baai      ########   odam.nl         */
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
	parseRequestLine();
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	if (!checkMethod())
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

bool httpHandler::checkMethod(void)
{
	for (auto &method : _request.loc.allowed_methods)
	{
		if (method == _request.method)
			return true;
	}
	setErrorResponse(eHttpStatusCode::MethodNotAllowed, "this method is not allowed for this location");
	return false;
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
		std::string newloc;
		std::string uri = _request.uri;
		std::string locpath = _request.loc.path;
		if (locpath == "/")
			locpath = "";
		uri.erase(0, locpath.length());
		newloc = _request.loc.redir_url + uri;
		_response.headers[eResponseHeader::Location] = newloc;
		_response.headers[eResponseHeader::ContentLength] = "0";
		return false;
	}
	return (true);
}

/**
 * @brief checks if the is a uri encoded request and if the path is valid
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
			return;
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
	if (_statusCode == eHttpStatusCode::Forbidden)
		return;
	if (_request.method == eHttpMethod::GET || _request.method == eHttpMethod::DELETE)
	{
		if (!std::filesystem::exists(_request.path))
		{
			return setErrorResponse(eHttpStatusCode::NotFound, "Resource not found at path: " + _request.path);
		}
		if (_request.method == eHttpMethod::GET)
		{
			if (_request.cgiReq == false && !std::filesystem::is_directory(_request.path) && access(_request.path.c_str(), X_OK) == 0)
			{
				return setErrorResponse(eHttpStatusCode::Forbidden, "attempt to GET executable file that doesn't match CGI");
			}
		}
	}
	if (_request.method == eHttpMethod::POST)
	{
		if (std::filesystem::exists(_request.path))
		{
			if (_request.cgiReq == false && !std::filesystem::is_directory(_request.path) && access(_request.path.c_str(), X_OK) == 0)
			{
				return setErrorResponse(eHttpStatusCode::Forbidden, "attempt to POST executable file that doesn't match CGI");
			}
		}
	}
	if (_request.cgiReq == true && !std::filesystem::is_directory(_request.path) && access(_request.path.c_str(), X_OK) != 0)
	{
		return setErrorResponse(eHttpStatusCode::Forbidden, "no permission to execute CGI file");
	}
}

/**
 * @brief builds the path.
 * Replaces the part of the uri that tags the location with the root of said location
 * if the file extension matches the cqi extension of the location it builds the path from cgi_path
 */
std::string httpHandler::buildPath(void)
{
	std::string outPath;
	std::string uri = _request.uri;
	std::string locpath = _request.loc.path;
	if (locpath == "/")
		locpath = "";
	if (!_request.loc.cgi_ext.empty())
	{
		size_t pos = _request.uri.find_last_of('.');
		if (pos != std::string::npos)
		{
			std::string extension = _request.uri.substr(pos);

			if (extension == _request.loc.cgi_ext)
			{
				return buildCgiPath();
			}
		}
	}
	// non-cgi
	if (!_request.loc.root.empty())
	{
		uri.erase(0, locpath.length());
		uri = _request.loc.root + uri;
	}
	else if (!_server.getRoot().empty())
	{
		uri.erase(0, locpath.length());
		uri = _server.getRoot() + uri;
	}
	outPath = "." + uri;
	return (outPath);
}

std::string httpHandler::buildCgiPath(void)
{
	std::string outPath;
	std::string uri = _request.uri;
	std::string locpath = _request.loc.path;
	if (locpath == "/")
		locpath = "";

	if (_request.loc.cgi_path.empty())
		return setErrorResponse(eHttpStatusCode::Forbidden, "no cgi_path provided for this location"), "";
	if (!_request.loc.root.empty())
	{
		uri.erase(0, locpath.length());
		uri = _request.loc.root + _request.loc.cgi_path + uri;
	}
	else if (!_server.getRoot().empty())
	{
		uri.erase(0, locpath.length());
		uri = _server.getRoot() + _request.loc.cgi_path + uri;
	}
	outPath = "." + uri;
	_request.cgiReq = true;
	return outPath;
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

/**
 * @brief sets transfer encoding, content length and content type
 */
void httpHandler::setContent(void)
{
	auto contentLengthHeader = findHeaderValue(_request, eRequestHeader::ContentLength);
	auto contentTypeHeader = findHeaderValue(_request, eRequestHeader::ContentType);
	auto transferEncodingHeader = findHeaderValue(_request, eRequestHeader::TransferEncoding);

	// setting tranferEncoding or Contentlength
	if (transferEncodingHeader.has_value())
	{
		if (transferEncodingHeader.value() == "chunked")
		{
			_request.body.chunked = true;
			_request.body.contentType = eContentType::contentLength;
		}
		else
		{
			_request.keepReading = false;
			_request.body.contentType = eContentType::error;
			return setErrorResponse(eHttpStatusCode::NotImplemented, "Transfer-Encoding type not implemented: " + transferEncodingHeader.value());
		}
	}
	else if (contentLengthHeader.has_value())
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
		_request.body.contentType = eContentType::contentLength;
	}
	else
	{
		_request.body.contentType = eContentType::noContent;
		return;
	}
	// setting contentType
	if (contentTypeHeader.has_value())
	{
		if (contentTypeHeader.value().find("multipart/form-data") != std::string::npos)
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
		else if (contentTypeHeader.value().find("application/") != std::string::npos)
		{
			_request.body.contentType = eContentType::application;
			return;
		}
		else if (contentTypeHeader.value().find("text/plain") != std::string::npos)
		{
			_request.body.contentType = eContentType::plain;
			return;
		}
		else
		{
			_request.keepReading = false;
			_request.body.contentType = eContentType::error;
			return setErrorResponse(eHttpStatusCode::NotImplemented, "contentType Header not implemented");
		}
	}
	_request.body.contentType = eContentType::undefined;
	return;
}
