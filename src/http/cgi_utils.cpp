/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   cgi_utils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/10 16:04:01 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/10 16:41:42 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/httpHandler.hpp"
#include "../../include/web.hpp"

void		closeAllPipes(int cgiIN[2], int cgiOUT[2])
{
	if (cgiIN[0])
		protectedClose(cgiIN[0]);
	if (cgiIN[1])
		protectedClose(cgiIN[1]);
	if (cgiOUT[0])
		protectedClose(cgiOUT[0]);
	if (cgiOUT[1])
		protectedClose(cgiOUT[1]);
}
