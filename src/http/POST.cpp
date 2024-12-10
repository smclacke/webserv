/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   POST.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 18:07:23 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 15:35:49 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

void httpHandler::stdPost(void)
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
		// Handle multipart form data using _request.body
		parseMultipartBody(contentType);
		if (_request.files.empty())
		{
			std::cerr << "No files found in request" << std::endl;
			setErrorResponse(eHttpStatusCode::BadRequest, "No files found in request");
			return;
		}
		else
		{
			// return the processed files
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
	else if (contentType == "application/x-www-form-urlencoded")
	{
		return wwwFormEncoded();
	}
	else if (contentType.find("application/") == 0) // change to any applicatoin type, send it straight to cgi and let it try to pass it to a program
	{
		if (_response.cgi == false)
			return setErrorResponse(eHttpStatusCode::Forbidden, "Cgi not allowed for this location");
		std::cout << "Received app data: " << _request.body.str() << std::endl;
		std::vector<char *> env;
		return cgiResponse(env);
	}
	else
	{
		std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
		setErrorResponse(eHttpStatusCode::UnsupportedMediaType, "Unsupported Content-Type: " + contentType);
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
}

void httpHandler::wwwFormEncoded(void)
{
	std::string filePath;
	if (std::filesystem::is_directory(_request.path))
	{
		filePath = _request.path + "/form_data.csv";
	}
	else if (std::filesystem::is_regular_file(_request.path))
	{
		if (_request.path.substr(_request.path.find_last_of(".") + 1) != "csv")
		{
			return setErrorResponse(eHttpStatusCode::UnsupportedMediaType, "Invalid file type: Only CSV files are supported");
		}
		filePath = _request.path;
	}
	else
	{
		return setErrorResponse(eHttpStatusCode::BadRequest, "Invalid path: Not a directory or a valid CSV file");
	}
	// check for perm
	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if (((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none) && (fileStatus.permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "No permission to read/write file: " + _request.path);
	}

	// Parse the form data directly from the stringstream
	std::string pair;
	std::map<std::string, std::string> formFields;

	while (std::getline(_request.body, pair, '&'))
	{
		size_t pos = pair.find('=');
		if (pos != std::string::npos)
		{
			std::string key = pair.substr(0, pos);
			std::string value = pair.substr(pos + 1);
			formFields[key] = value;
		}
	}

	// Write to CSV file
	std::ofstream csvFile(filePath, std::ios::out | std::ios::app);
	if (csvFile.is_open())
	{
		for (const auto &field : formFields)
		{
			csvFile << field.first << "," << field.second << "\n";
		}
		csvFile.close();
	}
	else
	{
		std::cerr << "Unable to open file for writing" << std::endl;
		return setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to write to CSV file");
	}

	_statusCode = eHttpStatusCode::OK;
	_response.headers[eResponseHeader::Location] = filePath;
	_response.body.str("Form data processed and stored successfully");
	_response.headers[eResponseHeader::ContentLength] = "44";
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