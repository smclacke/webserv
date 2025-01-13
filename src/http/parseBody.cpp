/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parseBody.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/21 15:40:36 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/10 16:40:04 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

//_request.bufferVec.insert(_request.bufferVec.end(), buffer, buffer + bytesRead);

void httpHandler::addToBody(char *buffer, size_t bytesRead)
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
		parseformData(buffer, bytesRead);
		return;
	}
	parseFixedLengthBody(buffer, bytesRead); // for regular content-len and applications / uri encoded
	return;
}

void httpHandler::parseFixedLengthBody(char *buffer, size_t bytesRead)
{
	if (_request.body.contentVec.size() + bytesRead >= _request.loc.client_body_buffer_size)
	{
		setErrorResponse(eHttpStatusCode::PayloadTooLarge, "body size exceeds client max body size");
		_request.keepReading = false;
	}
	_request.body.contentVec.insert(_request.body.contentVec.end(), buffer, buffer + bytesRead);
	if (_request.body.contentVec.size() >= _request.body.contentLen)
	{
		_request.body.content << std::string(_request.body.contentVec.data(), _request.body.contentVec.size());
		_request.keepReading = false;
	}
}

void httpHandler::parseformData(char *buffer, size_t bytesRead)
{
	if (_request.body.contentVec.size() + bytesRead >= _request.loc.client_body_buffer_size)
	{
		setErrorResponse(eHttpStatusCode::PayloadTooLarge, "body size exceeds client max body size");
		_request.keepReading = false;
	}
	_request.body.contentVec.insert(_request.body.contentVec.end(), buffer, buffer + bytesRead);
	// Check if the end delimiter is in the buffer
	static const std::vector<char> delimiterVec = [this]()
	{
		std::string delimiter = _request.body.formDelimiter + "--";
		return std::vector<char>(delimiter.begin(), delimiter.end());
	}();
	auto it = std::search(
		_request.body.contentVec.begin(),
		_request.body.contentVec.end(),
		delimiterVec.begin(),
		delimiterVec.end());
	if (it != _request.body.contentVec.end())
	{
		_request.body.content << std::string(_request.body.contentVec.data(), _request.body.contentVec.size());
		_request.keepReading = false;
		return;
	}
	if (_request.body.contentVec.size() >= _request.body.contentLen)
	{
		_request.body.content << std::string(_request.body.contentVec.data(), _request.body.contentVec.size());
		_request.keepReading = false;
		return;
	}
}

void httpHandler::parseChunkedBody(char *buffer)
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
				_request.body.content << std::string(_request.body.contentVec.data(), _request.body.contentVec.size());
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
			std::string chunkData = line.substr(0, dataStored);

			// Append chunk data to contentVec
			_request.body.contentVec.insert(_request.body.contentVec.end(), chunkData.begin(), chunkData.end());

			_request.body.nextChunkSize -= dataStored;
		}
	}
	_request.body.content << std::string(_request.body.contentVec.data(), _request.body.contentVec.size());
	_request.keepReading = false;
}
