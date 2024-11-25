/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/25 16:16:11 by jde-baai      ########   odam.nl         */
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
	if (_statusCode != eHttpStatusCode::OK)
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
	return;
}

void httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	return;
}