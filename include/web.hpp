/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   web.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 18:12:35 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/22 13:50:05 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include <cstring>
# include <iostream> // for cout
# include <unistd.h> // for read
# include <cstdlib> // for exit() + EXIT_FAILURE
# include <netinet/in.h> // for sockaddr_in
# include <sys/socket.h> // for socket functions


void		basicSocket();
