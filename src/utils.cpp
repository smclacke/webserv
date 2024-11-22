/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:06:02 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/22 23:57:34 by juliusdebaa   ########   odam.nl         */
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

