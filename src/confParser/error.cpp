/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   error.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/29 15:40:20 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/06 16:02:01 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/error.hpp"

/**
 * @brief Config file errors
 */
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
