/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 15:15:52 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

void httpHandler::stdResponse(void)
{
	std::cout << "It is a standard request" << std::endl;

	if (_request.method == eHttpMethod::GET)
		stdGet();
	else if (_request.method == eHttpMethod::POST)
		stdPost();
	else if (_request.method == eHttpMethod::DELETE)
		stdDelete();
	return;
}

/**
 * @note needs to be tested
 */
void httpHandler::stdGet(void)
{
	std::cout << "Handling GET request" << std::endl;

	// Check if the requested path is a directory
	if (std::filesystem::is_directory(_request.path))
	{
		if (_request.loc.autoindex)
		{
			generateDirectoryListing();
			return;
		}
		else
		{
			// Look for index files in the directory
			for (const auto &indexFile : _request.loc.index_files)
			{
				std::string indexPath = _request.path + "/" + indexFile;
				if (std::filesystem::exists(indexPath))
				{
					_request.path = indexPath;
					break;
				}
			}
		}
	}
	// Read the file content
	std::optional<std::string> fileContent = readFile(_request.path);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	// Set the response body
	if (fileContent.has_value())
		_response.body.str(fileContent.value());
	else
		return;
	_response.headers[eResponseHeader::ContentType] = contentType(_request.path);
	_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());

	return;
}

void httpHandler::stdPost(void)
{
	std::cout << "Handling POST request" << std::endl;
	// Add logic to handle POST request
	/*
	Purpose:
	The primary purpose of a POST request is to submit data to be processed to a specified resource. This could involve creating a new resource or updating an existing one.
	Request Body:
	Unlike GET requests, POST requests include a body that contains the data to be sent to the server. This data can be in various formats, such as JSON, XML, or form data.
	Idempotency:
	POST requests are not idempotent, meaning that making the same POST request multiple times may result in different outcomes (e.g., creating multiple resources).
	Headers:
	POST requests typically include headers that specify the content type of the request body (e.g., Content-Type: application/json).
	Response:
	The server's response to a POST request usually includes a status code indicating the result of the request. Common status codes include:
	201 Created: Indicates that the request was successful and a new resource was created.
	200 OK: Indicates that the request was successful, and the server processed the data.
	400 Bad Request: Indicates that the request was malformed or invalid.
	500 Internal Server Error: Indicates that the server encountered an error while processing the request.
	Location Header:
	If a new resource is created, the server may include a Location header in the response, specifying the URI of the newly created resource.
	*/
	// validation of the request
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
	}
	else if (contentType == "application/x-www-form-urlencoded")
	{
		// Determine the file path
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
		std::ofstream csvFile(filePath, std::ios::app);
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
	else if (contentType == "application/json") // change to any applicatoin type, send it straight to cgi and let it try to pass it to a program
	{
		if (_request.cgi == false)
			return setErrorResponse(eHttpStatusCode::Forbidden, "Cgi not allowed for this location");
		// Process JSON data
		// Example: Parse JSON and perform operations
		std::cout << "Received JSON data: " << _request.body.str() << std::endl;
		cgiResponse(); // add application/json to the call
		_statusCode = eHttpStatusCode::OK;
		_response.body.str() = "JSON data processed successfully";
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

void httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	return;
}