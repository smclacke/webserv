/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   DELETE.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 18:08:27 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/12 19:21:02 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

/**
 * @brief DELETE can delete files, delete wont call a CGI request
 */
void httpHandler::stdDelete(void)
{
	if (_request.cgiReq == true)
	{
		return setErrorResponse(eHttpStatusCode::Forbidden, "Not allowed to delete cgi programs");
	}
	if (_request.uriEncoded == true)
	{
		return setErrorResponse(eHttpStatusCode::NotImplemented, "Not accepting URI encoded delete requests");
	}
	if (std::filesystem::is_directory(_request.path))
		return setErrorResponse(eHttpStatusCode::MethodNotAllowed, "Not allowed to delete a directory");
	try
	{
		std::filesystem::remove(_request.path);
		_statusCode = eHttpStatusCode::NoContent;
		_response.body << "Succesfully deleted the requested file";
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Error deleting file: " + _request.path);
	}

	return;
}
