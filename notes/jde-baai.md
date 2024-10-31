
-- make the parseLocation function
-- In server.cpp write the functions to parse the values of the directives.


Questions:
1. should we store port as in_port_t instead of integer
2. should we change host from std::string to in_addr 

changes to implement this + setters and getters:

Server class:
private:
    in_port_t _port; // Change from into in_port_t
    struct in_addr _host; // Change from std::string to struct in_addr





