/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   error.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/18 14:04:41 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/09 13:49:37 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/error.hpp"

eConf::eConf(const std::string &msg, int line) : _msg(msg), _line(line)
{
}

eConf::eConf(const std::string &msg) : _msg(msg), _line(-1)
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
