/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:06:02 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/12 21:53:26 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

/* surely close failing is bad? how handle this? */
void protectedClose(int fd)
{
	if (fd != -1)
		close(fd);
	fd = -1;
	// if (fd < 0)
	//{
	//	std::cerr << "Invalid fd: " << fd << "\n";
	//	return ;
	// }
	// if (close(fd) == -1)
	//{
	//	std::cerr << "Failed to close fd: " << fd << " : " << strerror(errno) << "\n";
	//	throw std::runtime_error("Close failed");
	// }
}
