/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 17:47:42 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/06 17:49:59 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

void		protectedClose(int fd)
{
	if (fd)
	{
		if (close(fd) < 0)
			throw std::runtime_error("Close() failed\n");
	}
}
