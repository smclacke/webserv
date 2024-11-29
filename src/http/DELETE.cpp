/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   DELETE.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/28 18:08:27 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/28 18:46:04 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

void httpHandler::stdDelete(void)
{
	std::cout << "Handling DELETE request" << std::endl;
	// Add logic to handle DELETE request
	if (_request.uriEncoded == true)
	{
		return deleteFromCSV();
	}
	if (std::filesystem::is_directory(_request.path))
		return setErrorResponse(eHttpStatusCode::MethodNotAllowed, "Not allowed to delete a directory");
	// delete file, _request.path

	try
	{
		std::filesystem::remove(_request.path);
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		setErrorResponse(eHttpStatusCode::InternalServerError, "Error deleting file: " + _request.path);
	}
	return;
}

void httpHandler::deleteFromCSV()
{
	std::cout << "Deleting thing from database bruh" << std::endl;
}