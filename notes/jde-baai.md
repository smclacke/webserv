
-- make the parseLocation function
-- In server.cpp write the functions to parse the values of the directives.


Questions:
1. should we store port as in_port_t instead of integer
2. should we change host from std::string to in_addr 

changes to implement this:

Server class:
private:
    in_port_t _port; // Change from into in_port_t
    struct in_addr _host; // Change from std::string to struct in_addr


void Server::parseListen(std::stringstream &ss, int line_n)
{
    std::string value;
    std::string unexpected;
    ss >> value;
    if (ss >> unexpected)
        throw eConf("Unexpected value found: " + unexpected, line_n);
    size_t colonPos = value.find(':');
    if (colonPos == std::string::npos)
        throw eConf("Invalid listen directive: missing \':\'", line_n);
    std::string host = value.substr(0, colonPos);
    std::string portStr = value.substr(colonPos + 1);

    // checking the host IP for validity
    if (inet_pton(AF_INET, host.c_str(), &_host) <= 0) // Use inet_pton to convert IP
        throw eConf("Invalid host format. Expected 0.0.0.0", line_n);

    // checking the port number for validity
    if (portStr.length() != 4 || !std::all_of(portStr.begin(), portStr.end(), ::isdigit))
        throw eConf("Invalid port format. Expected 4 digits", line_n);
    _port = ntohs(static_cast<in_port_t>(std::stoi(portStr))); // Convert to network byte order
}


