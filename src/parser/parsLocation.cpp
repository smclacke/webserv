/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsLocation.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/31 15:42:05 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/11/01 12:54:29 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include <functional>

static void parseRoot(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string root;
	std::string unexpected;
	ss >> root;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.root = root;
}
static void parseClientBodyBufferSize(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string size;
	std::string unexpected;
	ss >> size;
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
		if (std::find(loc.accepted_methods.begin(), loc.accepted_methods.end(), e_method) != loc.accepted_methods.end())
			throw eConf("Double directive declared: " + method, line_n);
		loc.accepted_methods.push_back(e_method);
	}
}

static void parseRedirectUrl(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string url;
	std::string unexpected;
	ss >> url;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.redir_url = url;
}

static void parseRedirectStatus(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string numb;
	std::string unexpected;
	ss >> numb;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (numb.empty() || !std::all_of(numb.begin(), numb.end(), ::isdigit))
		throw eConf("Invalid redirect_status format. must be numerical", line_n);
	if (numb.size() != 3 && numb.at(0) != '3')
		throw eConf("Invalid redirect_status format. Expected 300,301,302..", line_n);
	loc.redirect_status = std::stoi(numb);
}

static void parseIndexFiles(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string file;
	while (ss >> file)
	{
		loc.index_files.push_back(file);
	}
}

static void parseAutoindex(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string status;
	std::string unexpected;
	ss >> status;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (status.empty() || status != "on" || status != "off")
		throw eConf("Not a valid autoindex value(on,off): ", line_n);
	if (status == "on")
		loc.autoindex = true;
	if (status == "off")
		loc.autoindex = false;
}

static void parseUploadDir(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string dir;
	std::string unexpected;
	ss >> dir;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.upload_dir = dir;
}

static void parseIndex(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string index;
	std::string unexpected;
	ss >> index;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.index = index;
}

static void parseCgiExt(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string ext;
	std::string unexpected;
	ss >> ext;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (ext.empty() || ext.size() < 2 || ext[0] != '.')
		throw eConf("Invalid cgi extension(must start with .): " + ext, line_n);
	loc.cgi_ext = ext;
}

static void parseCgiPath(std::stringstream &ss, int line_n, s_location &loc)
{
	std::string path;
	std::string unexpected;
	ss >> path;
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	loc.cgi_path = path;
}

void findLocationDirective(std::string &line, int &line_n, s_location &loc)
{
	if (line.back() == ';')
		line.pop_back();
	std::stringstream ss(line);
	std::string directive;
	ss >> directive;
	if (directive.empty())
		throw eConf("No directive found in line", line_n);

	auto dirMap = std::map<std::string, std::function<void(std::stringstream &, int, s_location &)>>{
		{"root", parseRoot},
		{"client_body_buffer_size", parseClientBodyBufferSize},
		{"accepted_methods", parseAcceptedMethods},
		{"redirect_url", parseRedirectUrl},
		{"redirect_status", parseRedirectStatus},
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