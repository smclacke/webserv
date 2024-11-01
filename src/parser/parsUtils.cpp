/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:24:19 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/01 14:08:24 by jde-baai      ########   odam.nl         */
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

static s_location parseLocation(std::ifstream &file, std::string &line, int &line_n)
{
	(void)file;
	s_location loc;
	std::stringstream ss(line);
	std::string location;
	std::string path;
	std::string bracket;
	std::string unexpected;

	ss >> location;
	ss >> path;
	ss >> bracket;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (location != "location")
		throw eConf("location directive not clear", line_n);
	if (*path.begin() != '/')
		throw eConf("Path does not start with /", line_n);
	if (bracket != "{")
		throw eConf("Unexpected value found: \'" + unexpected + "\' expeted: { ", line_n);
	loc.path = path;

	while (std::getline(file, line))
	{
		++line_n;
		lineStrip(line);
		if (line.empty())
			continue;

		if (line.find('}') != std::string::npos)
		{
			if (line.size() != 1)
				throw eConf("Unexpected text with closing }", line_n);
			break;
		}
		findLocationDirective(line, line_n, loc);
	}
	/* assigning defaults in case of no directives */
	if (loc.accepted_methods.size() == 0)
	{
		loc.accepted_methods.push_back(eHttpMethod::GET);
		loc.accepted_methods.push_back(eHttpMethod::POST);
		loc.accepted_methods.push_back(eHttpMethod::DELETE);
	}
	if (loc.index_files.size() == 0)
	{
		loc.index_files.push_back("index.html");
		loc.index_files.push_back("index.htm");
	}
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

	std::map<std::string, std::function<void(Server &, std::stringstream &, int)>> SerdirMap = {
		{"server_name", &Server::parseServerName},
		{"listen", &Server::parseListen},
		{"error_page", &Server::parseErrorPage},
		{"client_max_body", &Server::parseClientMaxBody},
	};
	if (SerdirMap.find(directive) == SerdirMap.end())
		throw eConf("Invalid directive found: " + directive, line_n);
	SerdirMap[directive](serv, ss, line_n);
}

/**
 * @note move to Server file
 */
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
		{
			if (line.size() != 1)
				throw eConf("Unexpected text with closing }", line_n);
			return (serv);
		}
		size_t pos = line.find("location");
		if (pos != std::string::npos)
		{
			s_location loc = parseLocation(file, line, line_n);
			if (loc.client_body_buffer_size > serv.getClientMaxBodySize())
				throw eConf("client_body_buffer_size exceeds server's maximum size limit", line_n);
			serv.addLocation(loc);
			continue;
		}
		findServerDirective(serv, line, line_n);
	}
	throw eConf("eof reached with no closing } for \"server\" keyword", line_n);
	return (serv);
}