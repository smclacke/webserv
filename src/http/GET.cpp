/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   GET.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 17:53:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/10 21:51:09 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @note needs to be tested
 */
void httpHandler::getMethod(void)
{
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
	// Check if the file is executable
	if (access(_request.path.c_str(), X_OK) == 0)
	{
		if (isCgi())
		{
			if (!generateEnv())
				return;
			return cgiResponse();
		}
		else
			return setErrorResponse(eHttpStatusCode::Forbidden, "file extension does not match accepted cgi extension");
	}
	else
	{
		readFile();
	}
	return;
}

/**
 * @brief calls cgi with the query paramaters if the query is valid
 */
void httpHandler::getUriEncoded(void)
{
	if (access(_request.path.c_str(), X_OK) == 0)
	{
		if (isCgi())
		{
			std::string queryEnv = "QUERY_STRING=" + _request.uriQuery;
			char *string = strdup(queryEnv.c_str());
			if (string == NULL)
				return setErrorResponse(eHttpStatusCode::InternalServerError, "malloc error");
			_cgidata.env.push_back(strdup(queryEnv.c_str()));
			if (!generateEnv())
				return;
			cgiResponse();
			return;
		}
		else
			return setErrorResponse(eHttpStatusCode::Forbidden, "file extension does not match accepted cgi extension");
	}
	else
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "url encoded request is does not target executable file");
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
		return setErrorResponse(eHttpStatusCode::Forbidden, "No permission to open file: " + _request.path);
	}
	std::string type = contentType(_request.path);
	auto acceptedH = findHeaderValue(_request, eRequestHeader::Accept);
	if (acceptedH.has_value())
	{
		if (acceptedH.value() != "*/*" && acceptedH.value().find(type) == std::string::npos)
		{
			return setErrorResponse(eHttpStatusCode::NotAcceptable, "File extension doesn't match the requested Accept header");
		}
	}
	// Read the file content
	_response.headers[eResponseHeader::ContentType] = type;
	openFile();
	return;
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

/**
 * @brief checks if the file on _request.path is executable and the extension is listed as cgi
 * sets ErrorResponse if its false
 */
bool httpHandler::isCgi()
{
	size_t pos = _request.path.find_last_of('.');
	if (pos != std::string::npos)
	{
		std::string extension = _request.path.substr(pos);
		if (extension == _request.loc.cgi_ext)
		{
			return true;
		}
		return false;
	}
	return false;
}