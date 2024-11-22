/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   stdRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/15 16:15:34 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/23 00:25:11 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"

std::string httpHandler::stdRequest(void)
{
	std::cout << "It is a standard request with these inputs:" << std::endl;
	return (writeResponse());
}