/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   GET.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 17:53:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/05 18:28:48 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @note needs to be tested
 */
void httpHandler::stdGet(void)
{
	std::cout << "Handling GET request" << std::endl;
	if (_request.uriEncoded == true)
	{
		return getUriEncoded();
	}
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
					std::string contentReturn = contentType(_request.path);
					break;
				}
			}
		}
	}
	else
	{
		// Check if the file is executable
		std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
		if ((fileStatus.permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
		{
			size_t pos = _request.path.find_last_of('.');
			if (pos != std::string::npos)
			{
				std::string extension = _request.path.substr(pos);
				if (extension != _request.loc.cgi_ext)
				{
					return setErrorResponse(eHttpStatusCode::Forbidden, "Executable request doesnt have the allowed cgi extension");
				}
			}
			_response.cgi = true;
			return;
		}
	}
	// check if file permission is readable.
	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		return setErrorResponse(eHttpStatusCode::Forbidden, "No permission to open file: " + _request.path);
	}
	std::string type = contentType(_request.path);
	auto AcceptedH = findHeaderValue(_request, eRequestHeader::Accept);
	if (AcceptedH.has_value())
	{
		if (AcceptedH.value().find(type) == std::string::npos)
		{
			return setErrorResponse(eHttpStatusCode::NotAcceptable, "File extension doesnt match the requested Accept header");
		}
	}
	// Read the file content
	_response.headers[eResponseHeader::ContentType] = type;
	readFile();
	return;
}

/**
 * @brief processes the URI encoded body, gets all the keys and tries to extract from a .csv file
 * @note replace this with CGI, send the uri encoded bit to a python script that retreives the value
 */
void httpHandler::getUriEncoded(void)
{
	std::cout << "URI encoded" << std::endl;
	if (_request.path.find(".") != std::string::npos)
	{
		if (_request.path.substr(_request.path.find_last_of(".") + 1) != ".csv")
			setErrorResponse(eHttpStatusCode::NotImplemented, "Only .csv is implemented for uri encoded");
	}
	else
		return setErrorResponse(eHttpStatusCode::BadRequest, "Path doesnt have file extension");

	std::string pair;
	std::map<std::string, std::string> params;
	while (std::getline(_request.body, pair, '&'))
	{
		size_t pos = pair.find('=');
		if (pos != std::string::npos)
		{
			std::string key = pair.substr(0, pos);
			std::string value = pair.substr(pos + 1);
			params[key] = value;
		}
	}

	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "No permission to read file: " + _request.path);
	}

	// Open the CSV file in input mode
	std::ifstream csvFile(_request.path, std::ios::in);
	if (!csvFile.is_open())
	{
		return setErrorResponse(eHttpStatusCode::InternalServerError, "unable to open file: " + _request.path);
	}
	std::string line;
	std::set<std::string> csvHeaders;
	if (std::getline(csvFile, line))
	{
		std::stringstream ss(line);
		std::string header;
		while (std::getline(ss, header, ','))
		{
			csvHeaders.insert(header);
		}

		// Check if all query keys exist in the CSV headers
		for (const auto &param : params)
		{
			if (csvHeaders.find(param.first) == csvHeaders.end())
			{
				setErrorResponse(eHttpStatusCode::BadRequest, "Key '" + param.first + "' does not exist in the CSV file.");
				return;
			}
		}
	}
	bool matchFound = false;
	std::string value;
	while (std::getline(csvFile, line))
	{
		std::stringstream ss(line);
		std::map<std::string, std::string> rowValues;
		for (const auto &header : csvHeaders)
		{
			if (!std::getline(ss, value, ','))
				break;
			rowValues[header] = value;
		}

		bool rowMatches = true;
		for (const auto &param : params)
		{
			if (rowValues[param.first] != param.second)
			{
				rowMatches = false;
				break;
			}
		}
		if (rowMatches)
		{
			matchFound = true;
			_response.body << value << "\n";
			break;
		}
	}
	if (!matchFound)
	{
		return setErrorResponse(eHttpStatusCode::NotFound, "No matching values found in the CSV file.");
	}
	return;
}

/**
 * @brief opens a pipe for the file and sets the outfile descriptor to _response.outFd
 */
void httpHandler::readFile(void)
{
	//int pipefd[2];
	//if (pipe(pipefd) == -1)
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to create pipe");
	//	return;
	//}

	//pid_t pid = fork();
	//if (pid == -1)
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to fork process");
	//	close(pipefd[0]);
	//	close(pipefd[1]);
	//	return;
	//}
	//if (pid == 0)
	//{					  // Child process
	//	close(pipefd[0]); // Close unused read end
	//	int fileFd = open(_request.path.c_str(), O_RDONLY);
	//	if (fileFd == -1)
	//	{
	//		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
	//		close(pipefd[1]);
	//		exit(EXIT_FAILURE);
	//	}
	//	exit(EXIT_FAILURE);			   // If execlp fails
	//}
	//else
	//{								  // Parent process
	//	close(pipefd[1]);			  // Close unused write end
	//	_response.readFd = pipefd[0]; // Set the read end of the pipe for epoll
	//	_response.pid = pid;
	//	_response.readFile = true;
	//}
	int out = open(_request.path.c_str(), O_RDONLY);
	if (out == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
		return;
	}
	//std::cout << "path =====" << _request.path << std::endl;
	//std::ifstream is(_request.path);
	//if (!is.is_open())
	//{
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
	//	return;	
	//}
	//std::string line;
	//std::getline(is, line); // if end of file returns false
	//if (is.fail())
	//{
	//	perror("bruh:");
	//	setErrorResponse(eHttpStatusCode::InternalServerError, "getline error");
	//	return;	
	//}
	//std::cout << "Read with getline =" << line << std::endl;
	//std::cout << "readfd in GET:" << out << std::endl;
	//_response.readFd = out;
	_response.filepath = _request.path;
	_response.readFd = out;
	_response.readFile = true;
}
