/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsebody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 18:43:50 by jde-baai      ########   odam.nl         */
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
	if (_request.uriEncoded == true)
	{
		size_t queryPos = _request.uri.find("?");
		if (queryPos == std::string::npos)
			setErrorResponse(eHttpStatusCode::InternalServerError, "Fault in setting uriEncoded requests");
		// Extract the query string
		std::string queryString = _request.uri.substr(queryPos + 1);

		// Store the query string in the _request.body stringstream
		_request.body.str(queryString);
	}

	std::optional<std::string> transferEncoding = findHeaderValue(_request, eRequestHeader::TransferEncoding);
	std::optional<std::string> contentLength = findHeaderValue(_request, eRequestHeader::ContentLength);

	if (transferEncoding.has_value())
	{
		if (transferEncoding.value() == "chunked")
		{
			parseChunkedBody(ss);
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
	std::optional<std::string> contentEncoding = findHeaderValue(_request, eRequestHeader::ContentEncoding);
	if (contentEncoding.has_value())
	{
		decodeContentEncoding(_request.body, contentEncoding.value());
	}
}

/**
 * @brief Reads a chunked body and adds it to _request.body
 */
void httpHandler::parseChunkedBody(std::stringstream &ss)
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
	_request.body << chunkedData.str();
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
