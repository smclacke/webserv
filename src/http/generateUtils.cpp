/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   generateUtils.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: juliusdebaaij <juliusdebaaij@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/24 11:28:30 by juliusdebaa   #+#    #+#                 */
/*   Updated: 2025/01/13 17:36:59 by smclacke      ########   odam.nl         */
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
			_response.body << entry.path().filename().string();
			if (html)
				_response.body << "<br>\r\n";
			else
				_response.body << "\r\n";
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
		{".ico", "image/*"},
		{".pdf", "application/pdf"},
		{".zip", "application/zip"},
		{".txt", "text/plain"}};
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

void httpHandler::CallErrorPage(std::string &path)
{
	// check if file permission is readable.
	std::filesystem::file_status fileStatus = std::filesystem::status(path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		std::cerr << "No permission to read error file" << path << std::endl;
		return;
	}
	std::string type = contentType(path);
	auto acceptedH = findHeaderValue(_request, eRequestHeader::Accept);
	if (acceptedH.has_value())
	{
		if (acceptedH.value() != "*/*" && acceptedH.value().find(type) == std::string::npos)
		{
			std::cerr << "Error file does not match accepted return values http" << std::endl;
		}
	}
	// Read the file content
	_response.headers[eResponseHeader::ContentType] = type;
	// open file
	try
	{
		_response.headers[eResponseHeader::ContentLength] = std::to_string(std::filesystem::file_size(path));
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		std::cerr << "Getting size of error file didnt work: " << e.what() << std::endl;
		return;
	}
	int fileFd = open(path.c_str(), O_RDONLY);
	if (fileFd == -1)
	{
		std::cerr << "Failed to open error file for some reason" << std::endl;
		return;
	}
	_response.readFile = true;
	_response.readFd = fileFd;
}

/**
 * @brief generates the environment based on the URI encoding
 * sets
 * _cgidata.scriptname
 * in _cgidata.env:
 * HTTP_ACCEPT
 * SERVER_NAME
 * USER_AGENT
 * REQUEST_METHOD
 * CONTENT_LENGTH
 * null_ptr
 */
bool httpHandler::generateEnv(void)
{
	try
	{
		std::optional<std::string> header;

		header = findHeaderValue(_request, eRequestHeader::Accept);
		if (header.has_value())
		{
			std::string accepted = "HTTP_ACCEPT=" + header.value();
			char *string = strdup(accepted.c_str());
			if (string == NULL)
				throw std::runtime_error("failed malloc");
			_cgi.env.push_back(string);
		}

		header = findHeaderValue(_request, eRequestHeader::Host);
		if (header.has_value())
		{
			std::string host = "SERVER_NAME=" + header.value();
			char *string = strdup(host.c_str());
			if (string == NULL)
				throw std::runtime_error("failed malloc");
			_cgi.env.push_back(string);
		}

		header = findHeaderValue(_request, eRequestHeader::UserAgent);
		if (header.has_value())
		{
			std::string userAgent = "HTTP_USER_AGENT=" + header.value();
			char *string = strdup(userAgent.c_str());
			if (string == NULL)
				throw std::runtime_error("failed malloc");
			_cgi.env.push_back(string);
		}

		size_t pos = _request.path.find_last_of('/');
		if (pos != std::string::npos)
		{
			std::string subPath = _request.path.substr(pos + 1);
			std::string scriptName = "SCRIPT_NAME=" + subPath;
			char *string = strdup(scriptName.c_str());
			if (string == NULL)
				throw std::runtime_error("failed malloc");
			_cgi.env.push_back(string);
			_cgi.scriptname = subPath;
		}

		std::string methodEnv = "REQUEST_METHOD=" + httpMethodToStringFunc(_request.method);
		char *string = strdup(methodEnv.c_str());
		if (string == NULL)
			throw std::runtime_error("failed malloc");
		_cgi.env.push_back(string);

		std::string length = "CONTENT_LENGTH=" + std::to_string(_request.body.content.str().size());
		char *clen = strdup(length.c_str());
		if (clen == NULL)
			throw std::runtime_error("failed malloc");
		_cgi.env.push_back(clen);

		_cgi.env.push_back(nullptr);
		//
	}
	catch (std::runtime_error &e)
	{
		std::cerr << "Error in generateEnv: " << e.what() << std::endl;
		setErrorResponse(eHttpStatusCode::InternalServerError, "malloc error");
		return false;
	}
	// set the httpOutput variable;
	/** @todo check this later **/
	//std::optional<std::string> acceptHeader = findHeaderValue(_request, eRequestHeader::Accept);
	//if (acceptHeader.has_value())
	//{
	//	if (acceptHeader.value().find("html") != std::string::npos)
	//	{
	//		_cgi.httpOutput = true;
	//	}
	//}
	//if (_request.loc.cgi_ext == ".cgi")
	//	_cgi.httpOutput = true;
	return true;
}

// Function to convert HTTP method enum to string
std::string httpMethodToStringFunc(eHttpMethod method)
{
	auto it = HttpMethodToString.find(method);
	if (it != HttpMethodToString.end())
	{
		return it->second;
	}
	return "Invalid";
}