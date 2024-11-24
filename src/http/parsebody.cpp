/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsebody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/24 11:19:37 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/server.hpp"
#include "../../include/web.hpp"

/**
 * @brief Parses the body of the HTTP request
 */
void httpHandler::parseBody(std::stringstream &ss)
{
	std::optional<std::string> contentType = findHeaderValue(_request, eRequestHeader::ContentType);
	std::optional<std::string> transferEncoding = findHeaderValue(_request, eRequestHeader::TransferEncoding);
	std::optional<std::string> contentEncoding = findHeaderValue(_request, eRequestHeader::ContentEncoding);
	std::optional<std::string> contentLength = findHeaderValue(_request, eRequestHeader::ContentLength);

	if (transferEncoding.has_value())
	{
		if (transferEncoding.value() == "chunked")
		{
			parseChunkedBody(ss, contentType);
		}
		else if (transferEncoding.value() == "identity" && contentLength.has_value())
		{
			parseFixedLengthBody(ss, std::stoul(contentLength.value()));
		}
		else
		{
			std::cerr << "Unsupported Transfer-Encoding: " << transferEncoding.value() << std::endl;
			_statusCode = eHttpStatusCode::NotImplemented;
			return;
		}
	}
	else if (contentType.has_value() && contentType.value().find("multipart/form-data") != std::string::npos)
	{
		parseMultipartBody(ss, contentType.value());
	}
	else if (contentLength.has_value())
	{
		parseFixedLengthBody(ss, std::stoi(contentLength.value()));
	}
	else
	{
		std::cerr << "No Content-Length or Transfer-Encoding header present" << std::endl;
		_statusCode = eHttpStatusCode::LengthRequired;
		return;
	}
	// decoding if it needs to be implemented
	if (contentEncoding.has_value())
	{
		decodeContentEncoding(_request.body, contentEncoding.value());
	}
}

/**
 * @brief Reads a chunked body and adds it to _request.body
 */
void httpHandler::parseChunkedBody(std::stringstream &ss, const std::optional<std::string> &contentType)
{
	std::string chunkSizeLine;
	size_t chunkSize;
	std::stringstream chunkedData;

	while (std::getline(ss, chunkSizeLine))
	{
		if (!chunkSizeLine.empty() && chunkSizeLine.back() == '\r')
			chunkSizeLine.pop_back();

		std::istringstream chunkSizeStream(chunkSizeLine);
		chunkSizeStream >> std::hex >> chunkSize;

		if (chunkSize == 0)
			break;

		std::string chunkData(chunkSize, '\0');
		ss.read(&chunkData[0], chunkSize);
		chunkedData << chunkData;
		std::getline(ss, chunkSizeLine); // Read the trailing \r\n
	}

	if (contentType.has_value() && contentType.value().find("multipart/form-data") != std::string::npos)
	{
		parseMultipartBody(chunkedData, contentType.value());
	}
	else
	{
		_request.body << chunkedData.str();
	}
}

/**
 * @note reads the body and adds to _request.body
 */
void httpHandler::parseFixedLengthBody(std::stringstream &ss, size_t length)
{
	if (length > _request.loc.client_body_buffer_size)
	{
		std::cerr << "Fixed length body size exceeds client body buffer size" << std::endl;
		_statusCode = eHttpStatusCode::InsufficientStorage;
		return;
	}
	std::string bodyLine;
	while (std::getline(ss, bodyLine) && bodyLine != "0\r")
	{
		if (!bodyLine.empty() && bodyLine.back() == '\r')
			bodyLine.pop_back();
		_request.body << bodyLine << "\n";
	}
	std::string bodyStr = _request.body.str();
	if (!bodyStr.empty() && bodyStr.back() == '\n')
	{
		bodyStr.pop_back();
		_request.body.str(bodyStr);
	}
}

void httpHandler::parseMultipartBody(std::istream &ss, const std::string &contentType)
{
	std::string boundary = extractBoundary(contentType);
	if (boundary.empty())
	{
		std::cerr << "Boundary not found in Content-Type" << std::endl;
		_statusCode = eHttpStatusCode::BadRequest;
		return;
	}

	std::string line;
	while (std::getline(ss, line))
	{
		if (line.find(boundary) != std::string::npos)
		{
			std::string headers;
			while (std::getline(ss, line) && !line.empty() && line != "\r")
			{
				headers += line + "\n";
			}

			std::string contentDisposition = extractHeaderValue(headers, "Content-Disposition");
			if (contentDisposition.find("filename=") != std::string::npos)
			{
				std::string filename = extractFilename(contentDisposition);
				std::string fileData;
				while (std::getline(ss, line) && line.find(boundary) == std::string::npos)
				{
					fileData += line + "\n";
				}
				saveFile(filename, fileData);
			}
		}
	}
}

std::string httpHandler::extractBoundary(const std::string &contentType)
{
	size_t pos = contentType.find("boundary=");
	if (pos != std::string::npos)
	{
		return "--" + contentType.substr(pos + 9);
	}
	return "";
}

std::string httpHandler::extractHeaderValue(const std::string &headers, const std::string &key)
{
	size_t pos = headers.find(key);
	if (pos != std::string::npos)
	{
		size_t start = headers.find(":", pos) + 1;
		size_t end = headers.find("\n", start);
		return headers.substr(start, end - start);
	}
	return "";
}

std::string httpHandler::extractFilename(const std::string &contentDisposition)
{
	size_t pos = contentDisposition.find("filename=");
	if (pos != std::string::npos)
	{
		size_t start = contentDisposition.find("\"", pos) + 1;
		size_t end = contentDisposition.find("\"", start);
		return contentDisposition.substr(start, end - start);
	}
	return "unknown";
}

void httpHandler::saveFile(const std::string &filename, const std::string &fileData)
{
	std::string uploadPath = "." + _request.loc.root + _request.loc.path + _request.loc.upload_dir;

	// Ensure the directory exists
	if (!std::filesystem::exists(uploadPath))
	{
		std::filesystem::create_directories(uploadPath);
	}

	std::ofstream outFile(uploadPath + "/" + filename, std::ios::binary);
	if (outFile)
	{
		outFile << fileData;
		outFile.close();
		std::cout << "File saved: " << filename << " to " << uploadPath << std::endl;
	}
	else
	{
		std::cerr << "Failed to save file: " << filename << std::endl;
		_statusCode = eHttpStatusCode::InternalServerError;
	}
}

/**
 * @brief Currently just returns if encoding is identity
 * @note in case encoding needs to be implemented - change this functoin
 */
void httpHandler::decodeContentEncoding(std::stringstream &body, const std::string &encoding)
{
	(void)body;
	if (encoding == "identity")
	{
		return;
	}
	// else if (encoding == "gzip")
	// {

	// 	// Implement gzip decoding logic here
	// }
	// else if (encoding == "deflate")
	// {
	// 	// Implement deflate decoding logic here
	// }
	// else if (encoding == "br")
	// {
	// 	// Implement Brotli decoding logic here
	// }
	else
	{
		std::cerr << "Unsupported Content-Encoding: " << encoding << std::endl;
		_statusCode = eHttpStatusCode::NotImplemented;
	}
}
