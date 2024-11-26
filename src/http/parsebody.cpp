/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsebody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/26 14:40:24 by jde-baai      ########   odam.nl         */
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
			return setErrorResponse(eHttpStatusCode::NotImplemented, "Unsupported Transfer-Encoding: " + transferEncoding.value());
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
		return setErrorResponse(eHttpStatusCode::LengthRequired, "No Content-Length or Transfer-Encoding header present");
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
	size_t totalSize = 0;

	while (std::getline(ss, chunkSizeLine))
	{
		if (!chunkSizeLine.empty() && chunkSizeLine.back() == '\r')
			chunkSizeLine.pop_back();

		std::istringstream chunkSizeStream(chunkSizeLine);
		chunkSizeStream >> std::hex >> chunkSize;

		if (chunkSize == 0)
			break;
		totalSize += chunkSize;
		if (totalSize > _request.loc.client_body_buffer_size)
		{
			std::cerr << "Chunked body size exceeds client max body size" << std::endl;
			return setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Chunked body size exceeds client max body size");
		}

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
 * @brief reads the body and adds to _request.body
 */
void httpHandler::parseFixedLengthBody(std::stringstream &ss, size_t length)
{
	if (length > _request.loc.client_body_buffer_size)
	{
		std::cerr << "Fixed length body size exceeds client body buffer size" << std::endl;
		return setErrorResponse(eHttpStatusCode::InsufficientStorage, "Fixed length body size exceeds client body buffer size");
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
	if (length != bodyStr.size())
		return setErrorResponse(eHttpStatusCode::BadRequest, "Content-Length mismatch");
}

void httpHandler::parseMultipartBody(std::istream &ss, const std::string &contentType)
{
	std::string boundary = extractBoundary(contentType);
	if (boundary.empty())
	{
		std::cerr << "Boundary not found in Content-Type" << std::endl;
		return setErrorResponse(eHttpStatusCode::BadRequest, "Boundary not found in Content-Type");
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
				std::string filePath = getTempFilePath(filename);
				std::ofstream outFile(filePath, std::ios::binary);
				if (!outFile)
				{
					std::cerr << "Failed to open file for writing: " << filePath << std::endl;
					setErrorResponse(eHttpStatusCode::InternalServerError, "Failed to save file: " + filename);
					return;
				}

				while (std::getline(ss, line) && line.find(boundary) == std::string::npos)
				{
					outFile << line << "\n";
					if (line.size() > _request.loc.client_body_buffer_size)
						return setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Line from multipart body exceeds client max body size");
				}
				outFile.close();
				_request.files.push_back(filePath); // Store file path for later use
				_statusCode = eHttpStatusCode::Created;
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

std::string httpHandler::getTempFilePath(const std::string &filename)
{
	if (_request.loc.root.empty())
		return ("." + _server.getRoot() + _request.loc.path + _request.loc.upload_dir + "/" + filename);
	else
		return ("." + _request.loc.root + _request.loc.path + _request.loc.upload_dir + "/" + filename);
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
		return setErrorResponse(eHttpStatusCode::NotImplemented, "Unsupported Content-Encoding: " + encoding);
	}
}
