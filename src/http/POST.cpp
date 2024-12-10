/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   POST.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 18:07:23 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 19:38:14 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

void httpHandler::postMethod(void)
{
	// std::cout << "Handling POST request" << std::endl;
	auto contentTypeIt = _request.headers.find(eRequestHeader::ContentType);
	if (contentTypeIt == _request.headers.end() || contentTypeIt->second.empty())
	{
		std::cerr << "Missing or empty Content-Type header" << std::endl;
		setErrorResponse(eHttpStatusCode::BadRequest, "Missing or empty Content-Type header");
		return;
	}
	const std::string &contentType = contentTypeIt->second;

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		return postMultiForm(contentType);
	}
	else if (contentType == "application/x-www-form-urlencoded")
	{
		return postUrlEncoded();
	}
	else if (contentType.find("application/") == 0) // change to any applicatoin type, send it straight to cgi and let it try to pass it to a program
	{
		return postApplication();
	}
	else
	{
		std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
		setErrorResponse(eHttpStatusCode::UnsupportedMediaType, "Unsupported Content-Type: " + contentType);
	}
	return;
}

// --------- multiform

/**
 * @brief processes a content-type multi-form-data post request
 */
void httpHandler::postMultiForm(const std::string &contentType)
{
	parseMultipartBody(contentType);
	if (_request.files.empty())
	{
		return;
	}
	else
	{
		_statusCode = eHttpStatusCode::Created;
		_response.headers[eResponseHeader::ContentType] = "text/plain";
		_response.body << "Files processed successfully: \r\n";
		for (auto file : _request.files)
		{

			if (file.find('/') != std::string::npos)
			{
				_response.body << file.substr(file.find_last_of('/') + 1) << "\r\n";
			}
			else
				_response.body << file << "\r\n";
			_response.headers[eResponseHeader::Location] = file;
		}
		_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
	}
	return;
}

void httpHandler::parseMultipartBody(const std::string &contentType)
{
	std::string boundary = extractBoundary(contentType);
	if (boundary.empty())
	{
		std::cerr << "Boundary not found in Content-Type" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Boundary not found in Content-Type");
	}
	std::string line;
	while (std::getline(_request.body, line))
	{
		if (line.find(boundary) != std::string::npos)
		{
			std::string headers;
			while (std::getline(_request.body, line) && !line.empty() && line != "\r")
			{
				headers += line + "\n";
			}
			std::string contentDisposition = extractHeaderValue(headers, "Content-Disposition");
			if (contentDisposition.find("filename=") != std::string::npos)
			{
				std::string filename = extractFilename(contentDisposition);
				std::string filePath = getTempFilePath(filename);
				std::ofstream outFile(filePath, std::ios::binary);
				if (!outFile)
				{
					std::cerr << "Failed to open file for writing: " << filePath << std::endl;
					setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to save file: " + filename);
					return;
				}
				while (std::getline(_request.body, line) && line.find(boundary) == std::string::npos)
				{
					outFile << line << "\n";
					if (line.size() > _request.loc.client_body_buffer_size)
						return setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Line from multipart body exceeds client max body size");
				}
				outFile.close();
				_request.files.push_back(filePath); // Store file path for later use
				_statusCode = eHttpStatusCode::Created;
			}
		}
	}
	if (_request.files.empty())
		return setErrorResponse(eHttpStatusCode::NoContent, "No \'filename=\' found in POST request");
}

std::string httpHandler::extractBoundary(const std::string &contentType)
{
	size_t pos = contentType.find("boundary=");
	if (pos != std::string::npos)
	{
		return "--" + contentType.substr(pos + 9);
	}
	return "";
}

std::string httpHandler::extractHeaderValue(const std::string &headers, const std::string &key)
{
	size_t pos = headers.find(key);
	if (pos != std::string::npos)
	{
		size_t start = headers.find(":", pos) + 1;
		size_t end = headers.find("\n", start);
		return headers.substr(start, end - start);
	}
	return "";
}

std::string httpHandler::extractFilename(const std::string &contentDisposition)
{
	size_t pos = contentDisposition.find("filename=");
	if (pos != std::string::npos)
	{
		size_t start = contentDisposition.find("\"", pos) + 1;
		size_t end = contentDisposition.find("\"", start);
		return contentDisposition.substr(start, end - start);
	}
	return "unknown";
}

std::string httpHandler::getTempFilePath(const std::string &filename)
{
	if (_request.loc.root.empty())
		return ("." + _server.getRoot() + _request.loc.path + _request.loc.upload_dir + "/" + filename);
	else
		return ("." + _request.loc.root + _request.loc.path + _request.loc.upload_dir + "/" + filename);
}

// ---------- url encoded

/**
 * @brief processes the Url encoded POST request
 */
void httpHandler::postUrlEncoded(void)
{
	if (_request.uriEncoded == false)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "Expected uri query, no ? found");
	if (!isExecutable())
		return setErrorResponse(eHttpStatusCode::Forbidden, "no executable rights or incorrect cgi extension");
	// set contentType
	std::string content = "CONTENT_TYPE=application/x-www-form-urlencoded";
	char *cstring = strdup(content.c_str());
	if (cstring == NULL)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc failed in postUrlEncoded");
	_cgi.env.push_back(cstring);
	// set Query string
	std::string queryEnv = "QUERY_STRING=" + _request.uriQuery;
	char *string = strdup(queryEnv.c_str());
	if (string == NULL)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc error");
	_cgi.env.push_back(strdup(queryEnv.c_str()));
	// set default env
	if (!generateEnv())
		return;
	return cgiResponse();
}

void httpHandler::postApplication(void)
{
	if (!isExecutable())
		return setErrorResponse(eHttpStatusCode::Forbidden, "no executable rights or incorrect cgi extension");
	// set contentType
	std::string content = "CONTENT_TYPE=application/x-www-form-urlencoded";
	char *cstring = strdup(content.c_str());
	if (cstring == NULL)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc failed in postUrlEncoded");
	_cgi.env.push_back(cstring);
	// set default env
	if (!generateEnv())
		return;
	return cgiResponse();
}
