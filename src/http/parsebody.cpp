/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsebody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/11 02:09:47 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void httpHandler::addToBody(std::string& buffer)
{
	if (_request.body.contentType == eContentType::noContent)
	{
		setErrorResponse(eHttpStatusCode::BadRequest, "unexpected body encountered");
		_request.keepReading = false;
		return;
	}
	if (_request.body.contentType == eContentType::contentLength)
	{
		parseFixedLengthBody(buffer);
		return;
	}
	if (_request.body.contentType == eContentType::formData)
	{
		parseformData(buffer);
		return;
	}
	if (_request.body.contentType == eContentType::chunked)
	{
		parseChunkedBody(buffer);
		return;
	}
	setErrorResponse(eHttpStatusCode::InternalServerError, "reached unexpected point");
	_request.keepReading = false;
	return;
}

void httpHandler::parseFixedLengthBody(std::string& buffer)
{
	_request.body.content << buffer;
	if (_request.body.content.str().size() >= _request.body.contentLen)
	{
		_request.keepReading = false;
	}
}

void httpHandler::parseformData(std::string& buffer)
{
	_request.body.content << buffer;
	if (_request.body.formDelimiter.empty())
	{
		size_t pos = buffer.find("\r\n");
		if (pos != std::string::npos)
		{
			//getting the delimiter from the start of the buffer
			_request.body.formDelimiter = buffer.substr(0, pos);
		}
	}
	// Check if the end delimiter is in the buffer
	if (buffer.find(_request.body.formDelimiter + "--") != std::string::npos)
	{
		_request.keepReading = false;
	}
}

void httpHandler::parseChunkedBody(std::string& buffer)
{
	std::string line;
	std::istringstream bufferStream(buffer);
	size_t chunkSize = 0;

	while (std::getline(bufferStream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (chunkSize == 0)
		{
			std::istringstream chunkSizeStream(line);
			chunkSizeStream >> std::hex >> chunkSize;

			if (chunkSize == 0)
			{
				_request.keepReading = false;
				return;
			}
			if (_request.body.totalChunked + chunkSize > _request.loc.client_body_buffer_size)
			{
				setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Chunked body size exceeds client max body size");
				_request.keepReading = false;
				return;
			}
		}
		else
		{
			// Read the chunk data
			std::string chunkData(chunkSize, '\0');
			bufferStream.read(&chunkData[0], chunkSize);
			_request.body.content << chunkData;

			_request.body.totalChunked += chunkSize;
			chunkSize = 0;
			// Read the trailing \r\n after the chunk data
			std::getline(bufferStream, line);
		}
	}
	_request.body.nextChunkSize = chunkSize;
}