/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   GET.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 17:53:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/13 15:34:30 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @note needs to be tested
 */
void httpHandler::getMethod(void)
{
	std::cout << "cgiReg bool = " << _request.cgiReq << "\n";
	if (_request.cgiReq == true)
	{
		generateEnv();
		cgiResponse();
	}
	// uri encoded GET request
	if (_request.uriEncoded == true)
	{
		return getUriEncoded();
	}
	// Check if the requested path is a directory
	if (std::filesystem::is_directory(_request.path))
	{
		if (!getDirectory())
			return;
	}
	readFile();
	return;
}

/**
 * @brief calls cgi with the query paramaters if the query is valid
 */
void httpHandler::getUriEncoded(void)
{
	if (_request.cgiReq == true)
	{
		std::string queryEnv = "QUERY_STRING=" + _request.uriQuery;
		char *string = strdup(queryEnv.c_str());
		if (string == NULL)
			return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc error");
		_cgi.env.push_back(strdup(queryEnv.c_str()));
		if (!generateEnv())
			return;
		cgiResponse();
		return;
	}
	else
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "url encoded request are only allowed as cgi");
		return;
	}
}

/**
 * @brief gets either the directory listing or index file
 * @return true if index file is found, otherwise false
 */
bool httpHandler::getDirectory(void)
{
	if (_request.loc.autoindex)
		generateDirectoryListing();
	else
	{
		for (const auto &indexFile : _request.loc.index_files)
		{
			std::string indexPath = _request.path + "/" + indexFile;
			if (std::filesystem::exists(indexPath))
			{
				_request.path = indexPath;
				return true;
			}
		}
		setErrorResponse(eHttpStatusCode::Forbidden, "Directory access is forbidden and no index file found.");
	}
	return false;
}

void httpHandler::readFile()
{
	// check if file permission is readable.
	if (access(_request.path.c_str(), R_OK) != 0)
	{
		return setErrorResponse(eHttpStatusCode::Forbidden, "No permission to read file: " + _request.path);
	}
	auto acceptedH = findHeaderValue(_request, eRequestHeader::Accept);
	if (acceptedH.has_value())
	{
		if (!isContentTypeAccepted(_request.path, acceptedH.value()))
		{
			return setErrorResponse(eHttpStatusCode::NotAcceptable, "File extension doesn't match the requested Accept header");
		}
	}
	// Read the file content
	_response.headers[eResponseHeader::ContentType] = contentType(_request.path);
	openFile();
	return;
}

bool httpHandler::isContentTypeAccepted(const std::string &filePath, const std::string &acceptHeader)
{
	std::string type = contentType(filePath);
	std::istringstream stream(acceptHeader);
	std::string mediaRange;
	while (std::getline(stream, mediaRange, ','))
	{
		mediaRange.erase(0, mediaRange.find_first_not_of(" \t"));
		mediaRange.erase(mediaRange.find_last_not_of(" \t") + 1);

		auto semicolonPos = mediaRange.find(';');
		std::string mediaType = (semicolonPos == std::string::npos) ? mediaRange : mediaRange.substr(0, semicolonPos);
		if (mediaType == "*/*" || mediaType == type || (mediaType.back() == '*' && type.find(mediaType.substr(0, mediaType.size() - 1)) == 0))
		{
			return true;
		}
	}
	return false;
}

/**
 * @brief opens the regular file, stores the fd in _response struct, gives this to handleWrite function
 * 	in epoll monitoring loop
 */
void httpHandler::openFile()
{
	try
	{
		_response.headers[eResponseHeader::ContentLength] = std::to_string(std::filesystem::file_size(_request.path));
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to retrieve file size: " + std::string(e.what()));
		return;
	}
	int fileFd = open(_request.path.c_str(), O_RDONLY);
	if (fileFd == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
		return;
	}
	_response.readFile = true;
	_response.readFd = fileFd;
}
