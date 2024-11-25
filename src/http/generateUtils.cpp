/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generateUtils.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: juliusdebaaij <juliusdebaaij@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/24 11:28:30 by juliusdebaa   #+#    #+#                 */
/*   Updated: 2024/11/24 11:58:14 by juliusdebaa   ########   odam.nl         */
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

std::string httpHandler::generateDirectoryListing(std::string &path)
{
	return (path);
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