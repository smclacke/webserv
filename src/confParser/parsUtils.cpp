/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:24:19 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/09 13:55:34 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/error.hpp"
#include "../../include/server.hpp"
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

/**
 * @brief checks if the paths(path, root, cgi_path) exist, otherwise throws eConf
 */
void Server::checkLocationPaths(s_location &loc, std::string const root, int const line_n)
{
	// check root path
	if (loc.root.empty() == false)
	{
		if (!std::filesystem::exists("." + loc.root)) // ignore the redline - compilation is fine
			throw eConf("Root directory \'" + loc.root + "\'does not exist", line_n);
		std::string combined;
		combined = "." + loc.root + loc.cgi_path; // check cgi path
		if (!std::filesystem::exists(combined))	  // ignore the redline - compilation is fine
			throw eConf("cgi directory \'" + combined + "\' does not exist", line_n);

		combined = "." + loc.root + loc.upload_dir; // check upload directory
		if (!std::filesystem::exists(combined))		// ignore the redline - compilation is fine
			throw eConf("Upload directory \'" + combined + "\' does not exist", line_n);
	}
	else
	{
		std::string combined;
		combined = "." + root + loc.path;		// check loc path
		if (!std::filesystem::exists(combined)) // ignore the redline - compilation is fine
			throw eConf("Path directory \'" + combined + "\' does not exist", line_n);

		combined = "." + root + loc.path + loc.cgi_path; // check cgi_path
		if (!std::filesystem::exists(combined))			 // ignore the redline - compilation is fine
			throw eConf("cgi directory \'" + combined + "\' does not exist", line_n);

		combined = "." + root + loc.path + loc.upload_dir; // check upload directory
		if (!std::filesystem::exists(combined))			   // ignore the redline - compilation is fine
			throw eConf("Upload directory \'" + combined + "\' does not exist", line_n);
	}
}

void Server::findServerDirective(Server &serv, std::string &line, int line_n)
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
