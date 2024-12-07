/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   error.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/29 14:36:48 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/18 14:01:14 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HPP
#define ERROR_HPP

#include "web.hpp"

class eConf : public std::exception
{
	private:
		std::string _msg;
		int _line;

	public:
		eConf(const std::string &msg, int line);
		const char *what() const throw();
		int line() const;
};

#endif /* ERROR_HPP */
