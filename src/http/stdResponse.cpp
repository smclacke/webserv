/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/25 11:05:34 by jde-baai      ########   odam.nl         */
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
		std::cout << "Is directory" << std::endl;
		if (_request.loc.autoindex)
		{
			// Generate and return a directory listing
			generateDirectoryListing(_request.path);
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

	// Check if the file exists
	if (!std::filesystem::exists(_request.path))
	{
		_statusCode = eHttpStatusCode::NotFound;
		return;
	}
	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		_statusCode = eHttpStatusCode::Forbidden;
		return;
	}

	// Read the file content
	std::string fileContent = readFile(_request.path);
	if (_statusCode != eHttpStatusCode::OK)
		return;

	// Set the response body
	_response.body.str(fileContent);
	_response.headers[eResponseHeader::ContentType] = contentType(_request.path);
	_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());

	return;
}

void httpHandler::stdPost(void)
{
	std::cout << "Handling POST request" << std::endl;
	// Add logic to handle POST request
	return;
}

void httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	return;
}