
-- make the parseLocation function
-- In server.cpp write the functions to parse the values of the directives.


Questions:
1. should we store port as in_port_t instead of integer
2. should we change host from std::string to in_addr 

check the parsing of the epoll() pages and check the data types if they match

changes to implement this + setters and getters:

Server class:
private:
    in_port_t _port; // Change from into in_port_t
    struct in_addr _host; // Change from std::string to struct in_addr





1. to do: check how paths work. Does root come before "path" or does "path" come first

- see which location has the largest amount of characters match with URI
- if locaiton root.empty() == false -> put root in front of the location path
else if server root.epmty() == false -> put server root in front of locaiton path
- try to see if that exists otherwise return bad request


2. In location parsing check if the directories actually exist otherwise return
3. After location parsing add the relative paths of root or path in front of the other potential paths in a location -depends on 1

4 Update the port and host to different datatypes in case those are requested by epoll();