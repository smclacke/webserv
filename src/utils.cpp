/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:06:02 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/11 21:07:09 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/web.hpp"

/* surely close failing is bad? how handle this? */
void		protectedClose(int fd)
{
	close(fd);
	//if (fd < 0)
	//{
	//	std::cerr << "Invalid fd: " << fd << "\n";
	//	return ;
	//}
	//if (close(fd) == -1)
	//{
	//	std::cerr << "Failed to close fd: " << fd << " : " << strerror(errno) << "\n";
	//	throw std::runtime_error("Close failed");
	//}
}
