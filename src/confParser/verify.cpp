/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   verify.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 16:40:38 by jde-baai      #+#    #+#                 */
/*   Updated: 2025/01/05 13:12:12 by julius        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/error.hpp"

/**
 * @brief checks if the location brackets are correct
 */
static void check_location_brackets(std::ifstream &file, std::string line, int &line_n)
{
	size_t pos = line.find("{");
	if (pos != std::string::npos) // found {
	{
		while (std::getline(file, line))
		{
			++line_n;
			lineStrip(line);
			if (line.empty())
				continue;
			if (line.find("{") != std::string::npos)
				throw eConf("unexpected \"{\" found", line_n);
			if (line.find("}") != std::string::npos)
				return;
		}
		throw eConf("eof reached with no closing } for \"location\" keyword", line_n);
	}
	throw eConf("no opening bracket after \"location\" keyword", line_n);
}

/**
 * @brief checks if the server brackets are correct.
 */
static void check_server_brackets(std::ifstream &file, std::string line, int &line_n)
{
	size_t pos = line.find("{");  // check if the server bracket has a {
	if (pos != std::string::npos) // found {
	{
		while (std::getline(file, line))
		{
			++line_n;
			lineStrip(line);
			if (line.empty())
				continue;
			size_t pos = line.find("location");
			if (pos != std::string::npos)
			{
				check_location_brackets(file, line.substr(pos + 8), line_n);
				continue;
			}
			if (line.find("{") != std::string::npos)
				throw eConf("unexpected \"{\" found", line_n);
			if (line.find("}") != std::string::npos)
				return;
		}
		throw eConf("eof reached with no closing } for \"server\" keyword", line_n);
	}
	throw eConf("no opening bracket after \"server\" keyword", line_n);
}

/** checks if every server and location block has an opening and a closing bracket */
void verifyInput(std::string &filename)
{
	if (filename.length() < 5 || filename.substr(filename.length() - 5) != ".conf")
		throw std::runtime_error("\"" + filename + "\" does not end with .conf");
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error("unable to open file: \"" + filename + "\"");

	/* checks if the server and location directives have the appropiate brackets */
	std::string line;
	int line_n = 0; // keeps track of the line_number for accurate error outputs
	while (std::getline(file, line))
	{
		++line_n;
		lineStrip(line);
		if (line.empty()) // skip empty lines
			continue;
		if (line.find('}') != std::string::npos) // random closing bracket = bad
			throw eConf("} bracket with no opening {", line_n);
		size_t pos;
		pos = line.find("Server");
		if (pos == std::string::npos)
			pos = line.find("server"); // check for server directive
		if (pos != std::string::npos)  // "server" keyword exists
		{
			check_server_brackets(file, line.substr(pos + 6), line_n);
			continue;
		}
		else
			throw eConf("line : \"" + line + "\": not recognized", line_n);
	}
	// eof reached - file should be ok :D
}
