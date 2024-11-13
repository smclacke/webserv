/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:24:19 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/13 14:39:00 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include <functional>

/**
 * @brief removes whitespace and anything that comes after a # from line
 */
void lineStrip(std::string &line)
{
	if (line.find('#') != std::string::npos)
		line.erase(line.find('#'), std::string::npos); // erase # and what comes afer
	if (line.find(';') != std::string::npos)
		line.erase(line.find(';'), std::string::npos);
	line.erase(0, line.find_first_not_of(" \t\n\r\f\v")); // erase start whitespace
	line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
}

void findServerDirective(Server &serv, std::string &line, int line_n)
{
	std::stringstream ss(line);
	std::string directive;
	ss >> directive;
	if (directive.empty())
		throw eConf("No directive found in line", line_n);

	std::map<std::string, std::function<void(Server &, std::stringstream &, int)>> SerdirMap = {
		{"server_name", &Server::parseServerName},
		{"listen", &Server::parseListen},
		{"error_page", &Server::parseErrorPage},
		{"client_max_body_size", &Server::parseClientMaxBody},
		{"root", &Server::parseRoot}};
	if (SerdirMap.find(directive) == SerdirMap.end())
		throw eConf("Invalid directive found: " + directive, line_n);
	SerdirMap[directive](serv, ss, line_n);
}
