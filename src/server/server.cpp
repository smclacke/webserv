/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 12:54:41 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/12/31 14:49:28 by juliusdebaa   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include "../../include/error.hpp"
#include "../../include/httpHandler.hpp"
#include <filesystem>

enum class SizeUnit
{
	kilobytes = 1,
	megabytes = 2,
	gigabytes = 3
};

/**
 * @brief	default constructor if no config file given
 */
Server::Server(void) : _port(8080)
{
	_serverName = "default_server";
	_host = "127.0.0.1";
	_errorPage.push_back({"/404.html", 404});
	_clientMaxBodySize = 10;
	s_location loc;
	loc.allowed_methods.push_back(eHttpMethod::GET);
	loc.allowed_methods.push_back(eHttpMethod::POST);
	loc.allowed_methods.push_back(eHttpMethod::DELETE);
	loc.index_files.push_back("index.html");
	loc.index_files.push_back("index.htm");
	loc.index = "index.html";
	loc.cgi_ext = ".php";
	loc.cgi_path = "/usr/bin/php-cgi";
	_location.push_back(loc);
	_serverSocket = std::make_shared<Socket>(*this);
}

Server &Server::operator=(const Server &rhs)
{
	if (this != &rhs)
	{
		_serverName = rhs._serverName;
		_host = rhs._host;
		_port = rhs._port;
		_errorPage = rhs._errorPage;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_location = rhs._location;
		_serverSocket = rhs._serverSocket;
	}
	return *this;
}

void Server::finalizeServerSetup(int line_n)
{
	if (_location.size() == 0)
		addLocation(addDefaultLoc(_clientMaxBodySize));
	checkErrorPages();
	if (_host.empty())
		throw eConf("No listen directive provided before end of Server", line_n);
	_serverSocket = std::make_shared<Socket>(*this);
	_logFile.open("./logs/" + _serverName + "_log.txt", std::ios::out | std::ios::app);
	if (!_logFile.is_open())
		throw std::runtime_error("Failed to open _log file for server: " + _serverName);
	logMessage("************************************\nServer class built for: " + _serverName + "\n************************************\n");
}

Server::Server(std::ifstream &file, int &line_n) : _serverName("Default_name"), _root("")
{
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
			finalizeServerSetup(line_n);
			return;
		}
		size_t pos = line.find("location");
		if (pos != std::string::npos)
		{
			s_location loc = parseLocation(file, line, line_n, _clientMaxBodySize);
			checkLocationPaths(loc, _root, line_n);
			if (loc.client_body_buffer_size > this->_clientMaxBodySize)
				throw eConf("client_body_buffer_size exceeds server's maximum size limit", line_n);
			this->addLocation(loc);
			continue;
		}
		findServerDirective(*this, line, line_n);
	}
	throw eConf("eof reached with no closing } for \"server\" keyword", line_n);
	return;
}

// destructor

Server::~Server(void)
{
	if (_logFile.is_open())
	{
		_logFile.close();
	}
}

/* member functions */

void Server::logMessage(const std::string &message)
{
	if (_logFile.is_open())
	{
		_logFile << message << std::endl;
	}
}

/**
 * @brief finds if the HHTP methods exists
 * @return invalid if it doesnt exist otherwise the Method
 */
eHttpMethod Server::allowedHttpMethod(std::string &str)
{
	if (str.empty())
		return (eHttpMethod::INVALID);
	auto it = StringToHttpMethod.find(str);
	if (it == StringToHttpMethod.end())
		return (eHttpMethod::INVALID);
	return (it->second);
}

void Server::printServer(void)
{
	std::cout << "Server Name: " << _serverName << std::endl;
	std::cout << "Host: " << _host << std::endl;
	std::cout << "Port: " << _port << std::endl;
	std::cout << "Root: " << _root << std::endl;
	std::cout << "Client Max Body Size: " << _clientMaxBodySize << "byte" << std::endl;
	std::cout << "Error Pages:" << _errorPage.size() << std::endl;
	for (const auto &errorPage : _errorPage)
	{
		std::cout << "  Path: " << errorPage.path << ", Code: " << errorPage.code << std::endl;
	}
	std::cout << "Locations:" << _location.size() << std::endl;
	for (const auto &location : _location)
	{
		std::cout << "  Path: " << location.path << std::endl;
		std::cout << "  Root: " << location.root << std::endl;
		std::cout << "  Client Body Buffer Size: " << location.client_body_buffer_size << std::endl;
		std::cout << "  Accepted methods: ";
		for (auto it = location.allowed_methods.begin(); it != location.allowed_methods.end(); ++it)
		{
			std::cout << HttpMethodToString.at(*it) << ", ";
		}
		std::cout << std::endl;

		std::cout << "  Redirect URL: " << (location.redir_url.empty() ? "None" : location.redir_url) << std::endl;
		std::cout << "  Redirect Status: " << (location.redirect_status ? std::to_string(location.redirect_status) : "None") << std::endl;

		std::cout << "  Index Files: ";
		if (!location.index_files.empty())
		{
			for (const auto &file : location.index_files)
			{
				std::cout << file;
				if (&file != &location.index_files.back())
				{
					std::cout << ", ";
				}
			}
		}
		else
		{
			std::cout << "None";
		}
		std::cout << std::endl;

		std::cout << "  Autoindex: " << (location.autoindex ? "Yes" : "No") << std::endl;
		std::cout << "  Upload Dir: " << (location.upload_dir.empty() ? "None" : location.upload_dir) << std::endl;
		std::cout << "  Index: " << (location.index.empty() ? "None" : location.index) << std::endl;
		std::cout << "  CGI Ext: " << (location.cgi_ext.empty() ? "None" : location.cgi_ext) << std::endl;
		std::cout << "  CGI Path: " << (location.cgi_path.empty() ? "None" : location.cgi_path) << std::endl;
		std::cout << "  -------- location end --------  " << std::endl;
	}
}

/* directives */

void Server::parseServerName(std::stringstream &ss, int line_n)
{
	std::string name;
	std::string unexpected;
	if (!(ss >> name))
		throw eConf("No value provided for directive", line_n);
	if (name.empty())
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	setServerName(name);
}

void Server::parseListen(std::stringstream &ss, int line_n)
{
	std::string value;
	std::string unexpected;
	if (!(ss >> value))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	if (value.empty())
		throw eConf("No value provided for directive", line_n);
	size_t colonPos = value.find(':');
	if (colonPos == std::string::npos)
		throw eConf("Invalid listen directive: missing \':\'", line_n);
	std::string host = value.substr(0, colonPos);
	std::string portStr = value.substr(colonPos + 1);

	// checking the host IP for validity
	std::istringstream ipStream(host);
	std::string part;
	int partCount = 0;
	while (std::getline(ipStream, part, '.'))
	{
		if (part.empty() || part.length() > 3 || !std::all_of(part.begin(), part.end(), ::isdigit))
			throw eConf("Invalid host format. Expected 0.0.0.0", line_n);
		int num = std::stoi(part);
		if (num < 0 || num > 255)
			throw eConf("Invalid host format. Each part must be between 0 and 255", line_n);
		partCount++;
	}
	if (partCount != 4)
		throw eConf("Invalid host format. Expected 4 parts", line_n);
	setHost(host);
	// checking the port number for validity
	if (portStr.length() != 4 || !std::all_of(portStr.begin(), portStr.end(), ::isdigit))
		throw eConf("Invalid port format. Expected 4 digits", line_n);
	setPort(std::stoi(portStr));
}

void Server::parseErrorPage(std::stringstream &ss, int line_n)
{
	std::string error_code;
	std::string path;
	std::string unexpected;
	if (!(ss >> error_code))
		throw eConf("No value provided for error_code", line_n);
	if (!(ss >> path))
		throw eConf("No value provided for path", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);

	if (error_code.length() != 3 || !std::all_of(error_code.begin(), error_code.end(), ::isdigit))
		throw eConf("Invalid error code format. Expected 3 digits", line_n);
	s_ePage nErrorPage;
	nErrorPage.code = std::stoi(error_code);
	if (nErrorPage.code <= 308 || nErrorPage.code > 511)
		throw eConf("Not a valid error code: " + error_code, line_n);
	nErrorPage.path = path;
	this->addErrorPage(nErrorPage);
}

void Server::checkErrorPages(void)
{
	for (auto &page : _errorPage)
	{
		std::string combinedPath = "." + _root + page.path;
		if (!std::filesystem::exists(combinedPath))
			throw eConf("error page directory does not exist: " + combinedPath, -1);
		page.path = combinedPath;
	}
}

void Server::parseClientMaxBody(std::stringstream &ss, int line_n)
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
	setClientMaxBodySize(maxBodySize);
}

void Server::parseRoot(std::stringstream &ss, int line_n)
{
	std::string root, rootpath;
	std::string unexpected;
	if (!(ss >> root))
		throw eConf("No value provided for directive", line_n);
	if (ss >> unexpected)
		throw eConf("Unexpected value found: " + unexpected, line_n);
	rootpath = "." + root;
	if (!std::filesystem::exists(rootpath))
		throw eConf("Root directory \'" + root + "\'does not exist", line_n);
	_root = root;
}

/* setters */
void Server::addLocation(s_location route)
{
	_location.push_back(route);
}

void Server::addErrorPage(s_ePage errorPage)
{
	_errorPage.push_back(errorPage);
}

void Server::setServerName(std::string serverName)
{
	_serverName = serverName;
}

void Server::setHost(std::string host)
{
	_host = host;
}

void Server::setPort(int port)
{
	_port = port;
}

void Server::setRoot(std::string root)
{
	_root = root;
}

void Server::setErrorPage(std::vector<s_ePage> errorPage)
{
	_errorPage = errorPage;
}

void Server::setClientMaxBodySize(size_t clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

void Server::setLocation(std::vector<s_location> location)
{
	_location = location;
}

void Server::setServerSocket(std::shared_ptr<Socket> serverSocket)
{
	_serverSocket = serverSocket;
}

/* getters */
const std::string &Server::getServerName(void) const
{
	return _serverName;
}

const std::string &Server::getHost(void) const
{
	return _host;
}

const int &Server::getPort(void) const
{
	return _port;
}

const std::string &Server::getRoot(void) const
{
	return _root;
}

const std::vector<s_ePage> &Server::getErrorPage(void) const
{
	return _errorPage;
}

const size_t &Server::getClientMaxBodySize(void) const
{
	return _clientMaxBodySize;
}

const std::vector<s_location> &Server::getLocation(void) const
{
	return _location;
}

std::shared_ptr<Socket> &Server::getServerSocket(void)
{
	return _serverSocket;
}

std::optional<s_ePage> Server::findErrorPage(int code) const
{
	for (auto &page : _errorPage)
	{
		if (page.code == code)
		{
			return page;
		}
	}
	return std::nullopt;
}
