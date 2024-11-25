/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generateUtils.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: juliusdebaaij <juliusdebaaij@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/24 11:28:30 by juliusdebaa   #+#    #+#                 */
/*   Updated: 2024/11/25 12:23:07 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

std::string httpHandler::readFile(std::string &filename)
{
	std::ifstream file(filename);
	std::ostringstream os;

	if (file.is_open())
	{
		os << file.rdbuf(); // Read the file's buffer into the output stream
		file.close();
	}
	else
	{
		_statusCode = eHttpStatusCode::InternalServerError;
	}

	return os.str(); // Return the contents as a string
}

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