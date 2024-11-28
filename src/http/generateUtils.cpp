/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generateUtils.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: juliusdebaaij <juliusdebaaij@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/24 11:28:30 by juliusdebaa   #+#    #+#                 */
/*   Updated: 2024/11/28 17:54:40 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief Attempts to read the specified file and returns its content as a string.
 *
 * This function checks if the file exists and if it has the necessary read permissions.
 * If the file is accessible, it reads the entire content into a string and returns it.
 * In case of any errors, such as the file not existing, lacking permissions, or failing to open,
 * it sets the appropriate HTTP status code and returns an empty optional.
 *.
 * @return An optional string containing the file content if successful, or std::nullopt on failure.
 */

void httpHandler::generateDirectoryListing(void)
{
	std::cout << "Generating directory listing" << std::endl;
	std::optional<std::string> acceptHeader = findHeaderValue(_request, eRequestHeader::Accept);
	bool html = false;
	if (acceptHeader.has_value())
	{
		if (acceptHeader.value().find("text/html") != std::string::npos)
		{
			html = true;
			_response.headers[eResponseHeader::ContentType] = "text/html";
		}
		else
			_response.headers[eResponseHeader::ContentType] = "text/plain";
	}
	else
		_response.headers[eResponseHeader::ContentType] = "text/plain";
	try
	{
		if (html)
		{
			_response.body << "<html><head><title>Directory Listing</title></head><body>";
			_response.body << "<h1>Directory Listing for " << _request.path << "</h1><ul>";
		}
		for (const auto &entry : std::filesystem::directory_iterator(_request.path))
		{
			_response.body << entry.path().filename().string() << "\n"; // List each file/directory name
		}
		if (html)
		{
			_response.body << "</ul></body></html>";
		}
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		_statusCode = eHttpStatusCode::InternalServerError;
		std::cerr << "Error returning Directory Listing" << std::endl;
		_response.body.str(""); // Clear the body
		if (html)
		{
			_response.body << "<html><body><h1>Error generating directory listing.</h1></body></html>";
		}
		else
		{
			_response.body << "Error generating listing";
		}
		_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
		return;
	}
	_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
}

std::string httpHandler::contentType(const std::string &filePath)
{
	static const std::unordered_map<std::string, std::string> mimeTypes = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".xml", "application/xml"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".png", "image/png"},
		{".gif", "image/gif"},
		{".svg", "image/svg+xml"},
		{".ico", "image/x-icon"},
		{".pdf", "application/pdf"},
		{".zip", "application/zip"},
		{".txt", "text/plain"},
	};
	// Find the last dot in the file path
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = filePath.substr(dotPos);
		auto it = mimeTypes.find(extension);
		if (it != mimeTypes.end())
		{
			return it->second;
		}
	}

	// Default MIME type if no match is found
	return "application/octet-stream";
}