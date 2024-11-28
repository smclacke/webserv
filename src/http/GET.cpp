/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   GET.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 17:53:29 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 18:53:26 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @note needs to be tested
 */
void httpHandler::stdGet(void)
{
	std::cout << "Handling GET request" << std::endl;
	if (_request.uriEncoded = true)
	{
		return getUriEncoded();
	}
	std::string contentReturn = contentType(_request.path);
	auto expected = findHeaderValue(_request, eRequestHeader::Accept);
	if (expected.has_value())
	{
		if (expected.value() != contentReturn)
		{
			return setErrorResponse(eHttpStatusCode::NotAcceptable, "File extension doesnt match the requested Accept header");
		}
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
					break;
				}
			}
		}
	}
	// Read the file content
	std::optional<std::string> fileContent = readFile(_request.path);
	if (_statusCode > eHttpStatusCode::Accepted)
		return;
	// Set the response body
	if (fileContent.has_value())
		_response.body.str(fileContent.value());
	else
		return;
	_response.headers[eResponseHeader::ContentType] = contentReturn;
	_response.headers[eResponseHeader::ContentLength] = std::to_string(_response.body.str().size());
	return;
}

/**
 * @brief processes the URI encoded body, gets all the keys and tries to extract from a .csv file
 */
void httpHandler::getUriEncoded(void)
{
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

std::optional<std::string> httpHandler::readFile(std::string &filename)
{
	if (!std::filesystem::exists(_request.path))
	{
		setErrorResponse(eHttpStatusCode::NotFound, "File to read doesn't exist: " + filename);
		return std::nullopt;
	}
	std::filesystem::file_status fileStatus = std::filesystem::status(_request.path);
	if ((fileStatus.permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
	{
		setErrorResponse(eHttpStatusCode::Forbidden, "No permission to open file: " + filename);
		return std::nullopt;
	}
	std::ifstream file(filename);
	std::ostringstream os;

	if (file.is_open())
	{
		os << file.rdbuf(); // Read the file's buffer into the output stream
		file.close();
	}
	else
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to open file: " + filename);
		return std::nullopt;
	}

	return os.str(); // Return the contents as a string
}