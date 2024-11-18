/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   error.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:04:41 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/18 14:17:51 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/error.hpp"

eConf::eConf(const std::string &msg, int line) : _msg(msg), _line(line)
{
}

const char *eConf::what() const throw()
{
	return _msg.c_str();
}

int eConf::line() const
{
	return _line;
}
