/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsLocation.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/31 15:42:05 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/19 18:05:28 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/error.hpp"
#include "../../include/server.hpp"
#include <functional>

/**
 * @note check if root dir actually exists
 */
static void parseRoot(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string root;
	std::string unexpected;
	if (!(ss >> root))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.root = root;
}
static void parseClientMaxBodySize(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string size;
	std::string unexpected;
	if (!(ss >> size))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);

	size_t maxBodySize = 0;
	if (size.empty() || !std::all_of(size.begin(), size.end() - 1, ::isdigit))
		throw eConf("Invalid size format. Expected a number followed by K, M, or G", line_n);
	if (size.back() == 'K' || size.back() == 'k')
		maxBodySize = std::stoi(size.substr(0, size.length() - 1)) * 1024;
	else if (size.back() == 'M' || size.back() == 'm')
		maxBodySize = std::stoi(size.substr(0, size.length() - 1)) * 1024 * 1024;
	else if (size.back() == 'G' || size.back() == 'g')
		maxBodySize = std::stoi(size.substr(0, size.length() - 1)) * 1024 * 1024 * 1024;
	else
		throw eConf("No size identifier found(m/g/k)", line_n);
	loc.client_body_buffer_size = maxBodySize;
}

static void parseAcceptedMethods(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string method;
	while (ss >> method)
	{
		auto it = StringToHttpMethod.find(method);
		if (it == StringToHttpMethod.end())
			throw eConf("Invalid HTTP method found: " + method, line_n);
		eHttpMethod e_method = it->second;
		if (std::find(loc.allowed_methods.begin(), loc.allowed_methods.end(), e_method) != loc.allowed_methods.end())
			throw eConf("Double directive declared: " + method, line_n);
		loc.allowed_methods.push_back(e_method);
	}
}

static void parseReturn(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string numb;
	std::string new_route;
	std::string unexpected;
	if (!(ss >> numb))
		throw eConf("No value provided for directive", line_n);
	if (!(ss >> new_route))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (numb.empty() || !std::all_of(numb.begin(), numb.end(), ::isdigit))
		throw eConf("Invalid redirect_status format. must be numerical", line_n);
	if (numb.size() != 3 && numb.at(0) != '3')
		throw eConf("Invalid redirect_status format. Expected 300,301,302..", line_n);
	loc.redirect_status = std::stoi(numb);
	loc.redir_url = new_route;
}

static void parseIndexFiles(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string file;
	if (!(ss >> file))
		throw eConf("No value provided for directive", line_n);
	while (ss >> file)
	{
		if (!file.empty())
			loc.index_files.push_back(file);
	}
}

static void parseAutoindex(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string status;
	std::string unexpected;
	if (!(ss >> status))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (status.empty() || (status != "on" && status != "off"))
		throw eConf("Not a valid autoindex value(on,off): ", line_n);
	if (status == "on")
		loc.autoindex = true;
	if (status == "off")
		loc.autoindex = false;
}

/**
 * @note check if the dir actually exists? add the dir to the root dir?
 */
static void parseUploadDir(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string dir;
	std::string unexpected;
	if (!(ss >> dir))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.upload_dir = dir;
}

static void parseIndex(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string index;
	std::string unexpected;
	if (!(ss >> index))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.index = index;
}

static void parseCgiExt(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string ext;
	std::string unexpected;
	if (!(ss >> ext))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (!ext.empty() && ext.front() == '"' && ext.back() == '"')
	{
		ext.erase(0, 1);
		ext.erase(ext.size() - 1, 1);
	}
	if (ext.empty() || ext.size() < 2 || ext.at(0) != '.')
		throw eConf("Invalid cgi extension(must start with .): " + ext, line_n);
	loc.cgi_ext = ext;
}

static void parseCgiPath(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string path;
	std::string unexpected;
	if (!(ss >> path))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.cgi_path = path;
}

void findLocationDirective(std::string &line, int &line_n, s_location &loc)
{
	std::stringstream ss(line);
	std::string directive;
	ss >> directive;
	if (directive.empty())
		throw eConf("No directive found in line", line_n);

	auto dirMap = std::map<std::string, std::function<void(std::stringstream &, int, s_location &)>>{
		{"root", parseRoot},
		{"client_max_body_size", parseClientMaxBodySize},
		{"allowed_methods", parseAcceptedMethods},
		{"return", parseReturn},
		{"index_files", parseIndexFiles},
		{"autoindex", parseAutoindex},
		{"upload_dir", parseUploadDir},
		{"index", parseIndex},
		{"cgi_ext", parseCgiExt},
		{"cgi_path", parseCgiPath}};

	if (dirMap.find(directive) == dirMap.end())
		throw eConf("Invalid directive found: " + directive, line_n);
	dirMap[directive](ss, line_n, loc);
}

/**
 * @note check if the location path exists
 * after perhaps replace path and root with just path if the root overwrite it but need to figure out how that works :)
 */
s_location parseLocation(std::ifstream &file, std::string &line, int &line_n, size_t maxbody)
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
	if (loc.client_body_buffer_size = std::numeric_limits<size_t>::max())
		loc.client_body_buffer_size = maxbody;
	if (loc.allowed_methods.size() == 0)
	{
		loc.allowed_methods.push_back(eHttpMethod::GET);
		loc.allowed_methods.push_back(eHttpMethod::POST);
		loc.allowed_methods.push_back(eHttpMethod::DELETE);
	}
	if (loc.index_files.size() == 0)
	{
		loc.index_files.push_back("index.html");
		loc.index_files.push_back("index.htm");
	}
	return (loc);
}

/*
allowed_methods:
GET / POST / DELETE
eHttpMethod

root
path

autoindex
on / off

index
index.html index.htm
if neither file is found server may return 403 Forbidden or 404 Not Found

upload_dir
path

client_max_body_size
same as other max_body_size parser

return -- redirects request from old route to new-route wiht a 301 status
301 /new-route

cgi_ext
.php / .pl / .py

-> only 1 per location possible

cgi_path -> allow only 1
path

*/