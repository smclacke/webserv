/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   verify.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jde-baai <jde-baai@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/23 16:40:38 by jde-baai      #+#    #+#                 */
/*   Updated: 2024/10/23 18:25:04 by jde-baai      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

bool check_location_brackets(std::ifstream &file, std::string line)
{
	return (true);
}

bool check_server_brackets(std::ifstream &file, std::string line)
{
	size_t pos = line.find("{");
	if (pos != std::string::npos) // found {
	{
		while (std::getline(file, line))
		{
			if (line.find("{") != std::string::npos)
				return (false);
			if (line.find("Location") != std::string::npos)
			{
				if (!check_location_brackets(file, line.substr(line.find("Location" + 8))))
					return (false);
				continue;
			}
			if (line.find("}") == std::string::npos)
				return (true); // found closing bracket }
		}
		return (false);
	}
	return (false); // no opening bracket
}

e_config verifyInput(int ac, char **av)
{
	/** check if argc is greater  */
	if (ac > 2)
	{
		std::cerr << "Error: too many inputs, provide 1 *.conf file" << std::endl;
		return (BAD_INPUT);
	}
	if (ac == 1)
		return (DEFAULT);
	std::string filename = std::string(av[1]);
	if (filename.substr(filename.length() - 5) != ".conf")
	{
		std::cerr << "Error: " << filename << " does not end with .conf" << std::endl;
		return (BAD_INPUT);
	}
	/* check if file opens and existence of at least one server{} directive */
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error: unable to open file " << filename << std::endl;
		return (BAD_INPUT);
	}
	e_config config = DEFAULT;
	std::string line;
	std::string server("server");
	while (std::getline(file, line))
	{
		line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
		line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
		if (line.empty() || line[0] == '#')
			continue;
		size_t pos = line.find(server);
		if (pos != std::string::npos) // "server" keyword exists
		{
			if (check_server_brackets(file, line.substr(pos + 6)))
			{
				config = SERVER_CONF;
				continue;
			}
			else
				return (BAD_INPUT);
		}
		else
			return (BAD_INPUT); // bad config - text input that is not server
	}
	return (config);
}
