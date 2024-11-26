/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/26 15:37:57 by jde-baai      ########   odam.nl         */
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
		// Handle multipart form data
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
					_response.body << file.substr(file.find_last_of('/') + 1) << "\r\n";
				else
					_response.body << file << "\r\n";
			}
			_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
		}
	}
	else if (contentType == "application/json")
	{
		if (_request.cgi == false || _request.loc.cgi_ext != ".json")
			return setErrorResponse(eHttpStatusCode::Forbidden, "Cgi not allowe for file extension: .json");
		// Process JSON data
		// Example: Parse JSON and perform operations
		std::cout << "Received JSON data: " << _request.body.str() << std::endl;
		cgiResponse(); // call json something
		_statusCode = eHttpStatusCode::OK;
		_response.body.str() = "JSON data processed successfully";
	}
	else if (contentType == "application/x-www-form-urlencoded")
	{
		// Process URL-encoded form data
		// std::cout << "Received form data: " << _request.body.str() << std::endl;
		// _statusCode = eHttpStatusCode::OK;
		// _response.body.str() = "Form data processed successfully";

		// std::cout << "Received form data: " << _request.body.str() << std::endl;

		// // Parse the form data
		// std::string formData = _request.body.str();
		// std::istringstream stream(formData);
		// std::string pair;
		// std::map<std::string, std::string> formFields;

		// while (std::getline(stream, pair, '&'))
		// {
		// 	size_t pos = pair.find('=');
		// 	if (pos != std::string::npos)
		// 	{
		// 		std::string key = pair.substr(0, pos);
		// 		std::string value = pair.substr(pos + 1);
		// 		formFields[key] = value;
		// 	}
		// }

		// // Example: Process the parsed data
		// for (const auto &field : formFields)
		// {
		// 	std::cout << "Field: " << field.first << ", Value: " << field.second << std::endl;
		// }

		// GET /retrieve-data?key1=value1&key2=value2 HTTP/1.1

		_statusCode = eHttpStatusCode::OK;
		_response.body.str() = "Form data processed successfully";
	}
	else
	{
		std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
		setErrorResponse(eHttpStatusCode::UnsupportedMediaType, "Unsupported Content-Type: " + contentType);
	}
	return;
}

void httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	return;
}