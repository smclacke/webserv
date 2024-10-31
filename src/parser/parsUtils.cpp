/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:24:19 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/31 12:38:17 by jde-baai      ########   odam.nl         */
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
		line.erase(line.find('#'), std::string::npos);	  // erase # and what comes afer
	line.erase(0, line.find_first_not_of(" \t\n\r\f\v")); // erase start whitespace
	line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
}

static void findLocationDirective(std::string &line, int &line_n, s_location &loc)
{
	(void)line;
	(void)line_n;
	(void)loc;
}

static s_location parseLocation(std::ifstream &file, std::string &line, int &line_n)
{
	(void)file;
	s_location loc;
	// parse line  for location route
	// loop through file and get the line -> findLocationDirective on the line
	// set each directive to the correct thingie in loc
	findLocationDirective(line, line_n, loc);
	return (loc);
}

static void findServerDirective(Server &serv, std::string &line, int line_n)
{
	if (line.back() == ';')
		line.pop_back();
	std::stringstream ss(line);
	std::string directive;
	ss >> directive;
	if (directive.empty())
		throw eConf("No directive found in line", line_n);

	std::map<std::string, std::function<void(Server &, std::stringstream &, int)>> dirMap = {
		{"server_name", &Server::parseServerName},
		{"listen", &Server::parseListen},
		{"error_page", &Server::parseErrorPage},
		{"client_max_body", &Server::parseClientMaxBody},
	};
	if (dirMap.find(directive) == dirMap.end())
		throw eConf("Invalid directive found: " + directive, line_n);
	dirMap[directive](serv, ss, line_n);
}

Server parseServer(std::ifstream &file, int &line_n)
{
	Server serv;
	std::string line;
	while (std::getline(file, line))
	{
		++line_n;
		lineStrip(line);
		if (line.empty())
			continue;
		if (line.find('}') != std::string::npos)
			return (serv);
		size_t pos = line.find("location");
		if (pos != std::string::npos)
		{
			serv.addLocation(parseLocation(file, line, line_n));
			continue;
		}
		findServerDirective(serv, line, line_n);
	}
	throw eConf("eof reached with no closing } for \"server\" keyword", line_n);
	return (serv);
}