/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/24 11:55:23 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

std::string httpHandler::stdRequest(void)
{
	std::cout << "It is a standard request with these inputs:" << std::endl;

	if (_request.method == eHttpMethod::GET)
		return (stdGet());
	else if (_request.method == eHttpMethod::POST)
		return (stdPost());
	else if (_request.method == eHttpMethod::DELETE)
		return (stdDelete());
	return (writeResponse());
}

std::string httpHandler::stdGet(void)
{
	std::cout << "Handling GET request" << std::endl;

	// Check if the requested path is a directory
	if (std::filesystem::is_directory(_request.path))
	{
		if (_request.loc.autoindex)
		{
			// Generate and return a directory listing
			return generateDirectoryListing(_request.path);
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

	// Check if the file exists
	if (!std::filesystem::exists(_request.path))
	{
		_statusCode = eHttpStatusCode::NotFound;
		return writeResponse();
	}
	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		_statusCode = eHttpStatusCode::Forbidden;
		return writeResponse();
	}

	// Read the file content
	std::string fileContent = readFile(_request.path);
	if (_statusCode != eHttpStatusCode::OK)
		return writeResponse();

	// Set the response body
	_response.body.str(fileContent);
	_response.headers[eResponseHeader::ContentType] = contentType(_request.path);
	_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());

	return writeResponse();
}

std::string httpHandler::stdPost(void)
{
	std::cout << "Handling POST request" << std::endl;
	// Add logic to handle POST request
	return (writeResponse());
}

std::string httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	return (writeResponse());
}