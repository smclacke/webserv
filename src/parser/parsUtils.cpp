/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:24:19 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/30 18:46:46 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

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

static std::map<std::string, void (Server::*)(std::stringstream &)> dirMap = {
	{"server_name", &Server::parseServerName},
	{"listen", &Server::parseListen},
	{"error_page", &Server::parseErrorPage},
	{"client_max_body_size", &Server::parseClientMaxBody}};

static void findLocationDirective(std::string &line, int &line_n)
{
	(void)line;
	(void)line_n;
}

static s_location parseLocation(std::ifstream &file, std::string &line, int &line_n)
{
	(void)file;
	s_location loc;
	findLocationDirective(line, line_n);
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
	if (dirMap.find(directive) == dirMap.end())
		throw eConf("Invalid directive found: " + directive, line_n);
	(serv.*dirMap[directive])(ss);
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