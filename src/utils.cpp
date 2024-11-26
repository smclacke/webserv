/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:06:02 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/26 16:01:23 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

/** @todo own catch/throw, close properly here, continue */
void		protectedClose(int fd)
{
	if (fd)
	{
		if (close(fd) < 0)
			throw std::runtime_error("Close() failed\n");
	}
}
