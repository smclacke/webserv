/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   POST.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 18:07:23 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/20 20:40:19 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

void httpHandler::postMethod(void)
{
	if (_request.body.contentType == eContentType::error)
	{
		setErrorResponse(eHttpStatusCode::BadRequest, "bad content");
	}
	if (_request.body.contentType == eContentType::noContent)
	{
		return generateEmptyFile();
	}
	else if (_request.body.contentType == eContentType::formData)
	{
		return postMultiForm();
	}
	else if (_request.uriEncoded == true)
	{
		return postUrlEncoded();
	}
	else if (_request.body.contentType == eContentType::application)
	{
		return postApplication();
	}
	else if (_request.body.contentType == eContentType::contentLength || _request.body.contentType == eContentType::plain || _request.body.contentType == eContentType::undefined)
	{
		return plainText();
	}
	setErrorResponse(eHttpStatusCode::NotImplemented, "POST request type not implemented");
	return;
}
// --------- multiform

void httpHandler::generateEmptyFile()
{
	std::fstream file;
	file.open(_request.path, std::ios::in | std::ios::out | std::ios::app); // Open for reading and writing, create if doesn't exist
	if (!file.is_open())
	{
		file.clear();							 // Clear any error flags
		file.open(_request.path, std::ios::out); // Create the file
		if (!file.is_open())
		{
			setErrorResponse(eHttpStatusCode::InternalServerError, "failed to open or create file");
			return;
		}
	}
	file.close();
	_response.body.clear();
	_response.body << "opened file";
	_statusCode = eHttpStatusCode::NoContent;
}

/**
 * @brief processes a content-type multi-form-data post request
 */
void httpHandler::postMultiForm(void)
{
	std::list<std::string> files;
	parseMultipartBody(files);
	if (_statusCode > eHttpStatusCode::PartialContent)
		return;
	else
	{
		_statusCode = eHttpStatusCode::Created;
		_response.headers[eResponseHeader::ContentType] = "text/plain";
		_response.body << "Files processed successfully: \r\n";
		for (auto file : files)
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

void httpHandler::parseMultipartBody(std::list<std::string> &files)
{
	if (_request.body.formDelimiter.empty())
	{
		std::cerr << "Boundary not found in Content-Type" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Boundary not found in Content-Type");
	}
	std::string line;
	bool fileStarted = false;
	std::ofstream outFile;
	std::string filePath;

	while (std::getline(_request.body.content, line))
	{
		if (line.find(_request.body.formDelimiter) != std::string::npos)
		{
			if (fileStarted)
			{
				outFile.close();
				files.push_back(filePath);
				fileStarted = false;
			}
			std::string headers;
			while (std::getline(_request.body.content, line) && !line.empty() && line != "\r")
			{
				headers += line + "\n";
			}
			std::string contentDisposition = extractHeaderValue(headers, "Content-Disposition");
			if (contentDisposition.find("filename=") != std::string::npos)
			{
				std::string filename = extractFilename(contentDisposition);
				filePath = getTempFilePath(filename);
				outFile.open(filePath, std::ios::binary);
				if (!outFile)
				{
					std::cerr << "Failed to open file for writing: " << filePath << std::endl;
					setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to save file: " + filename);
					return;
				}
				fileStarted = true;
			}
		}
		else if (fileStarted)
		{
			outFile << line << "\n";
			if (line.size() > _request.loc.client_body_buffer_size)
			{
				outFile.close();
				return setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Line from multipart body exceeds client max body size");
			}
		}
	}
	if (fileStarted)
	{
		outFile.close();
		files.push_back(filePath);
	}
	if (files.empty())
		return setErrorResponse(eHttpStatusCode::NoContent, "No 'filename=' found in POST request");
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
	return (_request.path + _request.loc.upload_dir + "/" + filename);
}

// ---------- url encoded

/**
 * @brief checks if the file is exists, is not a directory and is executable
 */
bool httpHandler::checkExecRights(void)
{
	if (!std::filesystem::exists(_request.path))
	{
		setErrorResponse(eHttpStatusCode::NotFound, "target file not found");
		return (false);
	}
	if (std::filesystem::is_directory(_request.path))
	{
		setErrorResponse(eHttpStatusCode::BadRequest, "executable POST request on directory");
		return (false);
	}
	if (access(_request.path.c_str(), X_OK) != 0)
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "the script you are trying to execute is not executable");
		return (false);
	}
	return (true);
}

/**
 * @brief processes the Url encoded POST request
 */
void httpHandler::postUrlEncoded(void)
{
	if (!checkExecRights())
		return;
	if (_request.uriEncoded == false)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "Expected uri query, no ? found");
	if (!_request.cgiReq)
		return setErrorResponse(eHttpStatusCode::Forbidden, "url encoded only allowed as cqi request");
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
	if (!checkExecRights())
		return;
	if (!_request.cgiReq)
		return setErrorResponse(eHttpStatusCode::Forbidden, "POST requests with content_type application/* only allowed as cqi request");
	auto contentOpt = findHeaderValue(_request, eRequestHeader::ContentType);
	if (!contentOpt.has_value())
	{
		return setErrorResponse(eHttpStatusCode::InternalServerError, "postApplication couldn't determinte contentType");
	}
	std::string content = "CONTENT_TYPE=" + contentOpt.value();
	char *cstring = strdup(content.c_str());
	if (cstring == NULL)
		return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc failed in postApplication");
	_cgi.env.push_back(cstring);
	// set default env
	if (!generateEnv())
		return;
	if (!_request.body.content.str().empty())
	{
		resetStringStream(_response.body);
		_response.body.str() = _request.body.content.str();
	}
	return cgiResponse();
}

void httpHandler::plainText(void)
{
	// cgi
	if (_request.cgiReq)
	{
		if (!generateEnv())
			return;
		if (!_request.body.content.str().empty())
		{
			resetStringStream(_response.body);
			_response.body.str() = _request.body.content.str();
		}
		return cgiResponse();
	}
	// non cgi
	std::ofstream outFile;
	outFile.open(_request.path, std::ios::out);
	if (!outFile.is_open())
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "couldn't open file");
		return;
	}
	outFile << _request.body.content.str();
	outFile.close();
	_response.body.clear();
	_statusCode = eHttpStatusCode::Created;
}
