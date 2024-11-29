/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:06:02 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/29 19:03:55 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

/** @todo own catch/throw, close properly here, continue */
bool		protectedClose(int fd)
{
	if (fd < 0)
	{
		std::cerr << "Invalid fd: " << fd << "\n";
		return false;
	}
	int result = close(fd);
	if (result < 0)
	{
		std::cerr << "Failed to close fd: " << fd << " : " << strerror(errno) << "\n";
		return false;
	}
	std::cout << "Successfully closed fd: " << fd << "\n";
	return true;
}

