/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   GET.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 17:53:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/05 18:43:37 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/epoll.hpp"

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

	int fileFd = open(_request.path.c_str(), O_RDONLY | O_NONBLOCK);
	if (fileFd == -1)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file");
		return;
	}

	// Add the file descriptor to epoll
	addToEpoll(fileFd);

	_response.readFd = fileFd;
	_response.readFile = true;
}

/*
void Epoll::processEvent(int fd, epoll_event &event)
{
	for (auto &serverData : _serverData)
	{
		if (fd == serverData._server->getServerSocket()->getSockfd())
		{
			if (event.events & EPOLLIN)
				makeNewConnection(fd, serverData);
		}
		for (auto &client : serverData._clients)
		{
			if (fd == client._fd)
			{
				if (event.events & EPOLLIN)
				{
					handleRead(client);
					if (client._clientState == clientState::READY)
					{
						modifyEvent(client._fd, EPOLLOUT);
						updateClientClock(client);
					}
				}
				else if (event.events & EPOLLOUT)
				{
					handleWrite(serverData, client);
					if (client._clientState == clientState::READY)
					{
						modifyEvent(client._fd, EPOLLIN);
						updateClientClock(client);
					}
				}
				else if (event.events & EPOLLHUP)
				{
					std::cout << "Epoll: EPOLLHUP\n";
					client._connectionClose = true;
				}
				else if (event.events & EPOLLRDHUP)
				{
					std::cout << "Epoll: EPOLLRDHUP\n";
					client._connectionClose = true;
				}
				else if (event.events & EPOLLERR)
				{
					std::cout << "EPoll: EPOLLERR\n";
					client._connectionClose = true;
				}
				if (client._connectionClose == true)
					handleClientClose(serverData, client);
			}
			else if (fd == client._responseClient.readFd)
			{
				// Handle file reading
				handleFileRead(client);
			}
		}
	}
}

void Epoll::handleFileRead(t_clients &client)
{
	char buffer[READ_BUFFER_SIZE];
	ssize_t bytesRead = read(client._responseClient.readFd, buffer, READ_BUFFER_SIZE - 1);
	if (bytesRead < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		std::cerr << "Reading from file failed\n";
		client._connectionClose = true;
		return;
	}
	else if (bytesRead == 0)
	{
		client._readingFile = false;
		client._clientState = clientState::READY;
		close(client._responseClient.readFd);
		client._responseClient.readFd = -1;
		if (client._responseClient.keepAlive == false)
			client._connectionClose = true;
		return;
	}
	buffer[bytesRead] = '\0';
	ssize_t bytesSend = send(client._fd, buffer, bytesRead, 0);
	if (bytesSend < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		std::cerr << "Write to client failed\n";
		client._connectionClose = true;
		return;
	}
}
*/