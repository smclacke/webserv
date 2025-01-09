/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsebody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/09 18:03:20 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void httpHandler::addToBody(std::string &buffer)
{
	if (_request.body.contentType == eContentType::noContent)
	{
		setErrorResponse(eHttpStatusCode::BadRequest, "unexpected body encountered");
		_request.keepReading = false;
		return;
	}
	if (_request.body.chunked == true)
	{
		parseChunkedBody(buffer);
		return;
	}
	if (_request.body.contentType == eContentType::formData)
	{
		parseformData(buffer);
		return;
	}
	parseFixedLengthBody(buffer); // for regular content-len and applications / uri encoded
	return;
}

void httpHandler::parseFixedLengthBody(std::string &buffer)
{
	_request.body.content << buffer;
	if (_request.body.content.str().size() >= _request.body.contentLen)
	{
		_request.keepReading = false;
	}
	if (_request.body.content.str().size() >= _request.loc.client_body_buffer_size)
	{
		setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Form data size exceeds client max body size");
		_request.keepReading = false;
	}
}

void httpHandler::parseformData(std::string &buffer)
{
	_request.body.content << buffer;
	if (_request.body.formDelimiter.empty())
	{
		_request.keepReading = false;
		return;
	}
	// Check if the end delimiter is in the buffer
	if (_request.body.content.str().find(_request.body.formDelimiter + "--") != std::string::npos)
	{
		_request.keepReading = false;
		return;
	}
	if (_request.body.content.str().size() >= _request.body.contentLen)
	{
		_request.keepReading = false;
		return;
	}
	if (_request.body.content.str().size() >= _request.loc.client_body_buffer_size)
	{
		setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Form data size exceeds client max body size");
		_request.keepReading = false;
	}
}

void httpHandler::parseChunkedBody(std::string &buffer)
{
	std::string line;
	std::istringstream bufferStream(buffer);

	while (std::getline(bufferStream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (_request.body.nextChunkSize == 0)
		{
			std::istringstream chunkSizeStream(line);
			if (!(chunkSizeStream >> std::hex >> _request.body.nextChunkSize))
			{
				setErrorResponse(eHttpStatusCode::BadRequest, "Invalid chunk size");
				_request.keepReading = false;
				return;
			}

			if (_request.body.nextChunkSize == 0)
			{
				_request.keepReading = false;
				return;
			}

			if (_request.body.totalChunked + _request.body.nextChunkSize > _request.loc.client_body_buffer_size)
			{
				setErrorResponse(eHttpStatusCode::PayloadTooLarge, "Chunked body size exceeds client max body size");
				_request.keepReading = false;
				return;
			}
		}
		else
		{
			// Read the actual chunk data
			size_t dataStored = (line.size() >= _request.body.nextChunkSize) ? _request.body.nextChunkSize : line.size();
			std::string chunkData = line.substr(0, _request.body.nextChunkSize);
			_request.body.content << chunkData;
			_request.body.nextChunkSize -= dataStored;
		}
	}
	_request.keepReading = false;
}
