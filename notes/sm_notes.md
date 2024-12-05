
#########################################################################

**GIT**


		<  git push -u origin <branch>  >

-u = --set-upstream


#########################################################################


**CHECK PORTS AND KILL IF LEFT OPEN AND LISTENING**

lsof -i :9999 - check open ports

kill <PID> - get PID from lsof info
kill -9 <PID> - alt if kill alone doesnt work


and if no PID but can't run, probably not being patient enough

#########################################################################

**weird ass server port/name/host shizzle**
Server name + host can be the same
	PORTS can never be reused

- server socket only created per port
BUT we will always have different ports and therefore, for us
	server = server socket everytime


*from eval sheet*

    In the configuration file setup multiple ports and use different websites. Use the browser to ensure that the configuration works as expected and shows the right website.
    In the configuration, try to setup the same port multiple times. It should not work.
    Launch multiple servers at the same time with different configurations but with common ports. Does it work? If it does, ask why the server should work if one of the configurations isn't functional. Keep going.


JanTheMan) NVM, JAN NOT RIGHT
	- servers can have same host and port IF different server_name
	- not all three host, port and name can be the same

#########################################################################


**GENERAL**

*blocking*

- blocking refers to situation where a thread is waiting for an operation to complete before it ca proceed further
	. this can lead to performance bottlesnecks, especially where the server needs to handle *multiple concurrent connections*
	. when dealing with input/output (I/O) operations (reading from or writing to sockets) - blocking issues often arise
	. traditionally, a thread will wait til data is available or write operation is done, waiting means the thread is inactive and server may struggle to handle other incoming requests
	
	solution: employ asynchronous I/O allowing server to initiate an I/O operation and continue with other tasks while waiting for operation to complete. a single thread can manage multiple connections simultaneously without being blocked

	- asynchronous I/O can be done using different system calls like selec,t poll, epoll (linux)
		. poll and epoll are simpler/better that select, but select is more portable (used across different platforms)

*HTTP 1.1*

- HTTP messages consist of a request or response line, headers, an empty line (CRLF or \r\n) and optional message body
	example:
		HTTP-message = start-line CRLF
						*(field-line CRLF)
						CRLF
						[ message-body]

		start0line 	 =	request-line /status-line 

	*HTPP/1.0* specification defined GET, HEAD, POST methods
	*HTTP/1.1* specification added PUT, DELETE, CONNECT, OPTIONS, TRACE

- get = requests the target resource transfer a representation of its state (HTTP status codes)
		requests using GET should only retrieve data without making changes
- head = asks for response identical to GET reuqest but without response body (only header)
- post = submits entity to the specified resource, often causing a change in state or side 
		effects on the server
- put = requests target resource create or update its state with the state defined by the
		submitted request, a distinction from POST is that the client specifies the target location on the server
- delete = deletes specified resource
- connect = establishes a tunnel to the server identifed by the target resource (?)
- options = requests that the target resource transfer the HTTP methods that it supports 
			can be used to check functionality of web server by requesting '*' instead of specific resource
- trace = requests target resource transfer received request in response body
			client can see what (if any) changes or additions have been made by intermediaries


- client handshake request:
	GET /chat HTTP/1.1
	Host: example.com:8000
	Upgrade: websocket
	Connection: Upgrade
	Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
	Sec-WebSocket-Version: 13

	*if any header is not understood or is incorrect, the server should send a 400 ("Bad Request") response and immediately close the socket*


- server handshake response:
	HTTP/1.1 101 Switching Protocols
	Upgrade: websocket
	Connection: Upgrade
	Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=

	- when the server receives the handshake request, is should send back a special response that indicates that the protocol will be changing from HTTP to WebSocket
	
	- server can additionally decide on extension/subprotocol requests here

	*seemingly overcomplicated process exists so that it's obvious to client whether the server supports WebSockets. important since security issues might arise if server accepts WebSockets connection bet interprets the data as a HTTP request*

	- if all good, handshake is complete and data can start being swapped


- keeping track of clients
	- server must keep track of clients' sockets so you don't keep handshaking again with clients who have already completed the handshake. *the same client IP address can try connecting multiple times*


- exchanging data frames
	- either client or server can choose to send a message at any time, ~WebSockets yay~
		however, extractinginfo from "frames" of data is not always a good experience.
		although all frames have same specific format, data going from client to server is masked using XOR encryption

~ doc continues with these topics: ~
	- format of data frames
	- decoding payload length
	- reading and unmasking data
	- message fragmentation
	- ping ponging
	- closing the connection


#########################################################################


**POll**

*from PDF*

Your server must never block and the client can be bounced properly if necessary.
•It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O
operations between the client and the server (listen included).
•poll() (or equivalent) must check read and write at the same time.
•You must never do a read or a write operation without going through poll() (or
equivalent).
•Checking the value of errno is strictly forbidden after a read or a write operation.
•You don't need to use poll() (or equivalent) before reading your configuration file


------------------------------------------------------
*poll monitor loop*
------------------------------------------------------

~~ source: https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-poll-monitor-activity-file-descriptors-message-queues

< int 	poll(void *listptr, nmsgsfds_t nmsgsfds, int timeout) >

- multiplexing input/output over these fds:
	. regular files
	. terminal + pseudoterminal devices
	. STREAMS-based files
	.sockets
	.message queues
	.FIFOs
	.pipes

*The poll() function is not affected by the O_NONBLOCK flag.*



#########################################################################


**CLIENTS AND SERVERS**

generally, all machines on the internet can be categorized as two types: servers and clients
machines that provide services (e.g. web servers) to other machines are *servers*
nachines used to connect to those services are *clients*

// yahoo example:
	- when you connect to www.yahoo.com to read a page, yahoo is providing a machine (probs cluster of large machines)
	for use on the internet, to service your request. yahoo provides a *server*
	- your machine, probs providing no services to anyone else on the internet, it is a user machine or *client*
	- a machine can be both a server and a client, mostly tho machines are one or the other
	- if running a browser, it will want to talk to the Web server on the server machine, Telnet application wants to talk to the Telnet
	server, email app talks to email server etc.

*server*
	-	server component provides a function or service to one or many clients which initiate requests for such services
	-	classified by the services they provide
	e.g. web server serves web pages and a file server serves computer files
	-	*the sharing of resources of a server constitutes a service*  
	shared resources may be any server computer's software, electronic components, programs, data, processors and storage devices


	*to formalize the data exchange even further, the server may implement an API, (application programming interface)*
		this is an abstraction layer for accessing a service, by restricting communication to a specific content format, it facilitates parsing. by abstracting access, it facilitates cross-platform data exchange (no idea what this means)


#########################################################################


**SOCKET_SERVER**


*SOCKET*

< int socket(int domain, int type, int protocol); >

 returns -1 on error
 returns file descriptor assigned to the socket

 domain - refers to protocol the socket will use for communication
 		possible values: AF_UNIX, AF_LOCAL (local)
			AF_INET (IPv4 internet), AF_INET6 (IPv6 internet), AF_IPx (IPX novell)

 type - specifies whether communication will be connectionless or persistent
		SOCK_STREAM (two-way reliable communication *TCP*)
		SOCK_DGRAM (connectionless, unreliable *UDP*)

 normally only one protocol available for each type, value 0 can be used


*BIND*

once there's a socket, we need to use bind to assign an IP address and port to the socket

< int	bind(int sockfd, const sockaddr *addr, socklen_t addrlen); >

	similar to socket, returns -1 on error, success 0

	sockfd - file descriptor we want to assign an address to, it will be the fd returned by socket

	addr - struct used to specify the address we want to assign to the socket
			exact struct to be used to define the address varies by protocol. 
			
			
		using IP - sockaddr_in

		struct sockaddr_in {
		sa_family_t    sin_family; /* address family: AF_INET */ - domain
		in_port_t      sin_port;   /* port in network byte order */ - port number
		struct in_addr sin_addr;   /* internet address */ - address for socket e.g. inet_addr("127.0.0.1")
		};

	addrlen - size() of addr


*LISTEN*

marks a socket as passive (socket will be used to accept connections)

< int	listen(int sockfd, int backlog); >

	returns -1 on error, success 0

	sockfd - fd of socket

	backlog - maximum number of connections that will be queued before connections start being refused


*ACCEPT*

extracts an element from a queue of connections (the queue created by listen) for a socket 

< int	accept(int sockfd, sockaddr *addr, socklen_t *addrlen); >

	returns -1 on error, returns fd for connection on success

	arguments are similar to bind() but addrlen is now value-result argument
		it expects a pointer to an int that will be the size of addr
		after the function is executed, the int refered by addrlen will be set to the size of the peer address

*READ | RECV*

receive a message from a socket
only diferent between read() and recv() is presence of flags

< ssize_t	recv(int sockfd, void *buf, size_t len, int flags) >
< ssize_t	read(int sockfd, void *buf, size_t len) >



*SETSOCKOPT*

< int	setsockopt(int sockfd, int level, int option_name, const void *option_value, socklen_t option_len) >

sets the option <option_name> argument, at the protocol level specified by the <level> argument, to the value <option_value> for socket sockfd


*FCNT manipulate file descriptor*

< int fcntl(int fd, int cmd, ... /* arg */) >

performs one of the operation <cmd> on open file descriptor <fd>


#########################################################################

*~ chatGPT ~*

i am building a simple webserver in C, is it correct that we could have multiple servers that have two sockets (client and server) and within these sockets there are many connections dependent on the requests being made by the client and server?


Yes, you're on the right track! In a typical web server setup in C, you can indeed have multiple servers, each with its own sockets for handling connections.


- Breakdown of Concepts:
Server Socket: This is a socket that listens for incoming client connections. You'll create this socket using functions like socket(), bind(), and listen().

- Client Sockets: Once a server socket accepts a connection (using accept()), it creates a new socket (often referred to as a client socket) for that specific connection. This allows the server to handle multiple clients simultaneously.

- Multiple Connections: Each server can handle multiple client connections concurrently. This is typically managed using:

- Forking: Creating a new process for each incoming connection.
Threads: Using threads to handle each connection.
Non-blocking I/O or select()/poll(): Using a single thread to manage multiple connections efficiently.


	Example Structure:
	Creating the Server Socket:

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	Binding and Listening:

	struct sockaddr_in server_addr;
	bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	listen(server_socket, SOMAXCONN);
	Accepting Connections:

	int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);


- Handling Multiple Connections: Depending on your chosen method (forking, threading, or I/O multiplexing), you will handle each client connection in a way that allows your server to continue accepting new connections while serving existing ones.



- Conclusion:
So yes, you can have multiple servers, each with their own server and client sockets, handling many connections based on the requests they receive. This architecture allows your web server to be scalable and efficient.


*~ second chatGPT ~*

so would i be creating multiple sockets depending on however many servers there are?

Yes, that's correct! If you have multiple server instances, each one would typically create its own server socket. Here’s how it generally works:


- Multiple Servers and Sockets:
Separate Server Instances: Each server instance would create its own listening socket. This allows each instance to handle requests independently.

- Creating Sockets: For each server:

You would call socket() to create a server socket.
Use bind() to associate the socket with a specific IP address and port.
Call listen() to prepare it for accepting incoming connections.
Client Connections: Each time a server instance accepts a connection (using accept()), it creates a new client socket for that connection. This allows the server to communicate with the client without blocking other connections.

	Example:
	If you have two server instances (let’s say on different ports), you would do something like this:

	Server Instance 1:
	c
	Copy code
	int server_socket1 = socket(AF_INET, SOCK_STREAM, 0);
	// Set up server_addr1 for instance 1
	bind(server_socket1, (struct sockaddr*)&server_addr1, sizeof(server_addr1));
	listen(server_socket1, SOMAXCONN);
	Server Instance 2:
	c
	Copy code
	int server_socket2 = socket(AF_INET, SOCK_STREAM, 0);
	// Set up server_addr2 for instance 2
	bind(server_socket2, (struct sockaddr*)&server_addr2, sizeof(server_addr2));
	listen(server_socket2, SOMAXCONN);


- Each server instance creates its own server socket for listening.
Each instance can accept multiple client connections, each handled through separate client sockets created upon accepting connections.
This setup allows for modular and scalable server architecture.

#########################################################################
#########################################################################

**GOOD EXPLANATION ROLES, SEVRERS, SOCKETS, CONNECTIONS**

When building a web server in C++, or in general working with socket-based communication, the server itself is the entity that manages and listens for incoming client connections, but the actual connections are established between the server and client sockets.

Breakdown of the roles of sockets and the server:

Server Socket:
	The server creates a server socket (also known as the listening socket) that listens for incoming connection requests from clients. This socket does not directly communicate with clients; it just waits for requests to come in.
	The server socket is typically created using the socket() system call and then bound to a specific IP address and port using bind().
	Once bound, the server socket starts listening for incoming client requests using listen().

Client Socket:
	Each client that wants to communicate with the server will initiate a connection. This can be done using the connect() system call on the client side to the server’s address and port.
	Once a connection request is received by the server socket, the server can accept the connection using accept(). This returns a new socket specifically for communication with that client.
	The client and server then communicate through this new socket, referred to as a client socket or communication socket. This socket is separate from the server's listening socket, which is just used to accept new connections.

Clarification of roles:
	Server: The server is responsible for listening for incoming connections, managing those connections, and handling requests. The server itself is what controls and manages the flow of data between the clients.
	Sockets: The sockets represent endpoints in the communication and are used to establish, maintain, and terminate connections between the server and the client.

	Multiple Servers:
	If you're working with multiple servers (e.g., handling multiple clients concurrently), you will generally have multiple server sockets (one per server), and each server socket can accept connections from multiple clients. Once a connection is accepted, a separate socket is created for that communication, and the server can continue listening for more incoming connections.

Example flow:
	The server listens for incoming connections using a server socket.
	A client connects to the server, and the server accepts the connection, creating a new client socket for communication.
	Data is transferred over the client socket.
	Once communication is done, the server closes the client socket, and the process is ready to accept new connections again.
	In summary:
	The server itself doesn't make connections; it listens for incoming requests and then handles those requests through client sockets once a connection is established.
	The sockets (client and server) are the actual objects that manage the connections, while the server coordinates the connections and communication.


###################################################################################################################################
###################################################################################################################################

**handling read + write in epoll loop**
**handling read + write in epoll loop**

recv() - reads data from socket, 0 if connection closed by peer, -1 error
if EAGAIN or EWOULDBLOCK - no data is currently available to read

read/write: used for file I/O streams
send/recv: socket communication


-------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------


#########################################################################
#########################################################################
#########################################################################


**MESS**
**MESS**
**MESS**


//// moving all these notes out of cpp files

/**
 * CLASS WEBSERV -
 * 
 * 	std::vector<Server>	_servers
 * 	Webserv(std::string config)
 * 
 * getserver(size_t index)
 * getserver(std::string name)
 * 
 * 
 * CLASS SERVER -
 * 
 * 	std::string _serverName
 * 	std::string _host
 * 	int		_port
 * // error page stuff
 * size_t	_clientMaxBodySize
 * std::vector<s_locaiton> 	_location
 * 
 * // setters inc: servername, host, pot, errorpage, clientmaxbodysize, location
 * Server(std::ifstream server_block);
 * 
 * 
 * STRUCT LOCATION -
 * 
 * 	std::string path
 * 	std::stirng root
 * 	size_t client_body_buffer_size
 * 	bools  = allow_GET allow_POST autoindex
 * std::string upload_dir
 * std::string index
 * std::string cgi_ext
 * std::string cgi_path
 * 
 * 
 * CONFIG EXAMPLE:
 * 	
 * 
 * 	location /files
 * 	autoindex on
 * 	alowed_methods GET
 * 	root /var/www/files
 * 	autoindex
 * 	upload_dir /var/uploads-local 
 * 
 */

 	//size_t additional_servers = 3;
	//for (size_t i = 0; i < additional_servers; ++i)
	//{
	//	Server nServer;
	//	wserv.addServer(nServer);
	//}
	//std::cout << "Created: " << wserv.getServerCount() << " servers" << std::endl;
	//Server getServer = wserv.getServer(0);
	//getServer.printServer();

// server	serv_instance;

//while (vector<servers>)
	// serv_instance = getserver(i)
	//_sockaddr.sin_port = serv_instance.getPort
	// sockfd = socket etc...

	//THEN
	// while serv.connections....
/*
Server serv;

while (vector<servers>)
{
	serv = webserv.getserver(0);
	_sockaddr.sin_port =serv.getPort; 
}

*/

// loopServers
//  -> per server - opensockets 
//	-> while opening sockets, listen for connections


Create an epoll instance: Use epoll_create1() to create an epoll instance.

Add sockets to epoll: Use epoll_ctl() to add your listening socket and any client sockets to the epoll instance for monitoring.

Wait for events: Use epoll_wait() to wait for events on the monitored file descriptors. This will tell you which sockets are ready for reading or writing.

Handle events: Once an event is detected, you can handle it accordingly, whether it’s accepting a new connection or reading data from a client.


int Socket::openServerSocket(int port) {
    // Create listening socket (similar to previous code)
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    // Set to non-blocking mode (similar to previous code)

    // Bind and listen (similar to previous code)

    // Create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "Error creating epoll instance\n";
        close(_sockfd);
        return -1;
    }

    // Add the listening socket to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN; // Monitor for incoming connections
    ev.data.fd = _sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _sockfd, &ev) == -1) {
        std::cerr << "Error adding socket to epoll\n";
        close(epoll_fd);
        close(_sockfd);
        return -1;
    }

    std::cout << "Listening on port " << port << "\n";

    while (true) {
        // Wait for events
        struct epoll_event events[10];
        int n = epoll_wait(epoll_fd, events, 10, -1);
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == _sockfd) {
                // Accept new connections
                socklen_t addrlen = sizeof(_sockaddr);
                int client_fd = accept(_sockfd, (struct sockaddr *)&_sockaddr, &addrlen);
                if (client_fd >= 0) {
                    // Set the client socket to non-blocking
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    // Add client socket to epoll
                    ev.events = EPOLLIN; // Monitor for incoming data
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                    std::cout << "Accepted new connection\n";
                }
            } else {
                // Handle data from clients
                char buffer[1000];
                ssize_t bytesRead = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::cout << "Received request: " << buffer << "\n";
                    // Send response (similar to previous code)
                } else {
                    // Handle client disconnect or error
                    close(events[i].data.fd);
                    std::cout << "Closed connection\n";
                }
            }
        }
    }

    close(epoll_fd); // Close the epoll instance
    close(_sockfd);  // Close the listening socket
    return 0;
}
Benefits of Using epoll()
Efficiency: epoll() is more efficient than traditional select() or poll(), especially with a large number of connections.
Scalability: It can handle a large number of file descriptors (up to thousands), making it suitable for high-traffic servers.
Edge-triggered and Level-triggered: You can choose how you want to receive notifications for events, which can optimize performance.
Next Steps
Implement Edge-triggered Mode: If you want to optimize further, you can use edge-triggered mode with EPOLLET to reduce the number of system calls.
Error Handling: Ensure robust error handling throughout your implementation.
Testing: Thoroughly test with multiple simultaneous clients to ensure stability.


----------- another explanation

If you want to manage both server and client sockets using a single epoll instance, you can do this by adding both the server socket (for accepting incoming connections) and the client socket (for communication with the server) to the same epoll loop.
Steps to Use One epoll for Both Server and Client

    Create a single epoll instance.
    Add both the server and client sockets to the epoll instance.
    Handle events for both sockets in the same event loop.

Here's how you can modify your code:
Step 1: Modify the Server Initialization

You'll create the epoll instance in your server's openServerSocket function and manage the client socket from there.
Step 2: Update the Code

Below is a modified version of your server function to manage both server and client sockets using a single epoll instance:

cpp

int Socket::openServerSocket(const Server &servInstance)
{
    // Create listening socket
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return (std::cout << "error socketing sock\n", -1);

    // Set to non-blocking socket mode
    _flags = fcntl(_sockfd, F_GETFL, 0);
    if (_flags == -1 || fcntl(_sockfd, F_SETFL, _flags | O_NONBLOCK) < 0)
    {
        close(_sockfd);
        return (std::cout << "error setting nonblocking\n", -1);
    }

    // Allow reuse of the address
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &_reuseaddr, sizeof(_reuseaddr)) < 0)
        return (std::cout << "error setsockopt sock\n", -1);

    memset(&_sockaddr, 0, sizeof(_sockaddr));
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    _sockaddr.sin_port = htons(servInstance.getPort());

    // Bind
    _addrlen = sizeof(_sockaddr);
    if (bind(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen) < 0)
    {
        close(_sockfd);
        return (std::cout << "error binding sock\n", -1);
    }

    // Listen for incoming connections
    if (listen(_sockfd, _maxConnections) < 0)
    {
        close(_sockfd);
        return (std::cerr << "error listening for connections\n", -1);
    }

    std::cout << "listening successfully on port - " << servInstance.getPort() << " \n";

    // Create an epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "error creating epoll instance\n";
        return -1;
    }

    // Add server socket to epoll instance
    struct epoll_event event;
    event.events = EPOLLIN; // Monitor for incoming connections
    event.data.fd = _sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _sockfd, &event) < 0) {
        std::cerr << "error adding server socket to epoll\n";
        return -1;
    }

    // Initialize client socket (you can set this to an invalid value initially)
    int client_socket = -1;

    // Main event loop
    while (true)
    {
        struct epoll_event events[10]; // Array to hold events
        int nfds = epoll_wait(epoll_fd, events, 10, -1); // Wait for events

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == _sockfd) {
                // Accept new connection
                _connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
                if (_connection < 0) {
                    std::cerr << "error accepting connection\n";
                    continue;
                }
                std::cout << "successfully made connection\n";

                // Set the new connection socket to non-blocking
                _flags = fcntl(_connection, F_GETFL, 0);
                fcntl(_connection, F_SETFL, _flags | O_NONBLOCK);

                // Add new connection to epoll instance
                event.events = EPOLLIN | EPOLLET; // Edge-triggered
                event.data.fd = _connection;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _connection, &event) < 0) {
                    std::cerr << "error adding connection to epoll\n";
                    close(_connection);
                }
            } else {
                // Handle incoming data from a client connection
                int client_fd = events[i].data.fd;
                char buffer[1024];
                ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::cout << "received request: " << buffer << "\n";

                    // Send HTTP response
                    std::string response = generateHttpResponse("get me on your browser");
                    send(client_fd, response.c_str(), response.size(), 0);
                } else {
                    if (bytesRead < 0) {
                        std::cerr << "error reading from client\n";
                    }
                    // Close the client socket
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr); // Remove from epoll
                }
            }
        }
    }

    close(epoll_fd);
    return 1;
}

Handling Client Connections

You can have a separate function for the client, but instead of creating a new epoll instance, simply use the existing one:

    In openClientSocket: After you connect the client, you can add the client socket to the same epoll instance. This means you'll have to pass the epoll_fd to your client function.

    Event Loop: The same event loop can handle events for both the server and the client.

Example of Handling Client in the Same Loop

You could manage sending a request to the server from your client side by triggering events that notify the epoll loop about the client's socket. This may involve additional state management to keep track of the request and response.
Conclusion

By consolidating everything into a single epoll instance, you can efficiently manage both server and client sockets in the same event loop. This can simplify your code and improve performance, especially when dealing with multiple connections. Just ensure your logic can differentiate between events coming from server and client sockets so that you handle them appropriately.


// something else


	// SOMETHING WEIRD AND CONFUSING
	//while (true)
	//{
	//	//_events[MAX_EVENTS];
	//	_numEvents = epoll_wait(_epfd, events, MAX_EVENTS, -1);
		
	//	for (int i = 0; i < _numEvents; ++i)
	//	{
	//		if (_events[i].data.fd == _sockfd)
	//		{	
	//			// accept connection
	//			_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
	//			if (_connection < 0)
	//			{
	//				if (errno == EWOULDBLOCK || errno == EAGAIN)
	//					continue ;
	//				std::cerr << "error accepting connection\n";
	//				continue ;
	//			}
	//			// TODO: set connection to non-blocking

	//			// add client to epoll (need to check this in connection with opening the client socket)
	//			_clientEvent.events = EPOLLIN;
	//			_clientEvent.data.fd = _sockfd; // client fd - how to separate ...
	//			epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &_clientEvent);
				
	//		}
	//		else	
	//			// handle data from client socket
	//			// client_fd = events[i].data.fd;
	//			// read and process data
	//	}
	//}

		// FROM SERVER SOCKET FUNCTION
	// epoll comes in here | moving while loop into epoll.monitoring method
	//while (true)
	//{	
	//	// accept connection
	//	_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
	//	if (_connection < 0)
	//	{
	//		if (errno == EWOULDBLOCK || errno == EAGAIN)
	//			continue ;
	//		std::cerr << "error accepting connection\n";
	//		continue ;
	//	}

	//	std::cout << "successfully made connection\n";

	//	// read from the connection
	//	_bufferSize = sizeof(_buffer);
	//	while ((_bytesRead = read(_connection, _buffer, _bufferSize - 1)) > 0)
	//	{
	//		_buffer[_bytesRead] = '\0';
	//		_request += _buffer;
	//		if (_request.find("\r\n\r\n") != std::string::npos)
	//			break ; // end of HTTP request
	//	}
		
	//	if (_bytesRead < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	//		std::cerr << "error reading from connection\n";

	//	if (!_request.empty())
	//	{
	//		std::cout << "received request: " << _request << "\n";

	//		// send HTTP response
	//		std::string	_response = generateHttpResponse(std::string("get me on your browser"));
	//		if (send(_connection, _response.c_str(), _response.size(), 0) < 0)
	//			std::cerr << "error snding response to client\n";
	//	}
	//	else
	//		std::cout << "received empty request\n";

	//	close(_connection);
	//}


				else if (events[i].events & EPOLLIN)
			{
				// read data from socket
				char		buffer[1024];
				int			bytesRead = 0;
				std::string	request;
				
				while (read(events[i].data.fd, buffer, sizeof(buffer) - 1) < 0)
				{
					buffer[bytesRead] = '\0';
					request += buffer;
					if (request.find("\r\n\r\n") != std::string::npos)
						break ; // end of HTTP request
				}
				if (bytesRead <= 0)
				{
					close (events[i].data.fd);
					std::cout << "Client disconnected\n";
				}
				if (!request.empty())
				{
					std::cout << "Server received request: " << request << "\n";
					
					// send HTTP response
					std::string	response = generateHttpResponse("here, have a response");

					if (send(events[i].data.fd, response.c_str(), response.size(), 0) < 0)
						std::cerr << "Error sending response to client\n";
				}
				else
					std::cout << "received empty request\n";
			}
	



				//ssize_t 	bytesRead = read(clientSockFd, buffer, sizeof(buffer) - 1);
				//if (bytesRead > 0)
				//{
				//	buffer[bytesRead] = '\0';
				//	std::cout << "received request: " << buffer << "\n";
					
				//	// send HTTP response
				//	std::string	response = generateHttpResponse("here, have a response");
				//	send(clientSockFd, response.c_str(), response.size(), 0);
				//	std::cout << "successfully read message from client to server\n";
				//}
				//else
				//{

/**
 * monitoring serveral sockets
 * waits for changing state or changing level for each socket monitored
 * handles lots of socket descriptors
 * 
 * contains internal structure: interest list (corresponding to all FDs monitors), a ready list (corresponding to FDs ready for I/O)
 * 
 * 
 * ---- functions ----
 * 
 * creates new epoll instance and reutrns a descriptor
 * < int		epoll_create(int nb); >
 * 
 * 
 * changes the behaviour of epoll instance
 * epfd: descriptor of epoll instance created
 * op: operation wanted on epoll structure (e.g. add new fd in interest list, modify/delete)
 * fd: concerned descriptor
 * event: filled with concerned fd and flags to apply to fd
 * < int		epoll_ctl(int epfd, int op, int fd, struct epoll_event *event); >
 * 
 * 
 * 
 * waits for event on any descriptor in interest list
 * funtion will block until - fd delivers event | call is interrupted by signal handler | timeout expires
 * epfd: descriptor of epoll instance created
 * maxevents: max of events returned
 * events: return info from ready list
 * < int		epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout); >
 * 
 * 
 * 
 */

#########################################################################
#########################################################################

OLD WHILE FROM WHEN THINGS WERE NICER + EASIER


while (true)
	{	
		// accept connection
		_addrlen = sizeof(_sockaddr);
		_connection = accept(_sockfd, (struct sockaddr *)&_sockaddr, &_addrlen);
		if (_connection < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue ;
			std::cerr << "error accepting connection\n";
			continue ;
		}

		// i thought i needed nonblocking on connection but this breaks it
		// set connection to nonblocking
		//int flags2 = fcntl(_connection, F_GETFL, 0);
		//if (flags2 == -1 || fcntl(_connection, F_SETFL, flags2 | O_NONBLOCK) < 0)
		//{
		//	close(_connection);	
		//	continue ;
		//}

		std::cout << "successfully made connection\n";

		// read from the connection
		char	buffer[1000];
		std::string	request;
		ssize_t bytesRead;
		while ((bytesRead = read(_connection, buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0';
			request += buffer;
			if (request.find("\r\n\r\n") != std::string::npos)
				break ; // end of HTTP request
		}
		
		if (bytesRead < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
			std::cerr << "error reading from connection\n";

		if (!request.empty())
		{
			std::cout << "received request: " << request << "\n";

			// send HTTP response
			std::string	response = 
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 27\r\n"
				"Connection: close\r\n"
				"\r\n"
				"response message from server!";
				
			if (send(_connection, response.c_str(), response.size(), 0) < 0)
				std::cerr << "error snding response to client\n";
		}
		else
			std::cout << "received empty request\n";

		close(_connection);


OLD CLIENT SOCKET FUNCTION

	// create client socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (std::cout << "error socketing sock\n", -1);

	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons(servInstance.getPort());
	inet_pton(AF_INET, servInstance.getHost().c_str(), &_sockaddr.sin_addr);

	// set to non-blocking socket mode
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags == -1 || fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(_sockfd);
		return (std::cout << "error setting client socket to nonblocking\n", -1);
	}

	// attempt to connect
	_addrlen = sizeof(_sockaddr);
	if ((connect(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen)) < 0)
	{
		close(_sockfd);
		return (std::cout << "error connecting to server from client\n", -1);
	}
	std::cout << "client connected successfully to port - " << servInstance.getPort() << " \n";
	
	// send message to server socket 
	std::string message = "GET / HTTP/1.1\r\nHost: " + servInstance.getHost() + "\r\nConnection: close\r\n\r\n";
	if (send(_sockfd, message.c_str(), message.size(), 0) < 0)
	{
		close(_sockfd);
		return (std::cout << "error sending from client\n", -1);
	}
	
	// read response from server
	char	buffer[100];
	ssize_t bytesRead;
	while ((bytesRead = read(_sockfd, buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytesRead] = '\0';
		std::cout << "received response: " << buffer;
	}
	if (bytesRead < 0)
		std::cerr << "error reading reponse\n";
	buffer[99] = '\0';
	std::cout << "read by client: <" << buffer << ">\n";
	

***MESSY***
***MESSY***
***MESSY***
***MESSY***



// moving out the way
	
	// SERVER ADDRESS AND PORT
	//memset(&_sockaddr, 0, sizeof(_sockaddr));
	//_sockaddr.sin_family = AF_INET;
	//_sockaddr.sin_port = htons(servInstance.getPort());
	//inet_pton(AF_INET, servInstance.getHost().c_str(), &_sockaddr.sin_addr);
	//setSockfd(_sockfd);


	//_host = servInstance.getHost();
	//setHost(_host);
	

	// CONNECT

	///* TESTING OLD CLIENT FUNCTION FROM HERE */
	//// attempt to connect
	//if ((connect(_sockfd, (struct sockaddr *)&_sockaddr, _addrlen)) < 0)
	//{
	//	close(_sockfd);
	//	throw std::runtime_error("Error connecting to server from client\n");
	//}
	//std::cout << "Client connected successfully to port - " << servInstance.getPort() << " \n";

	//std::string message = "GET / HTTP/1.1\r\nHost: " + servInstance.getHost() + "\r\nConnection: close\r\n";
	//if (send(_sockfd, message.c_str(), message.size(),0) < 0)
	//{
	//	close(_sockfd);
	//	throw std::runtime_error("Error sending message from client\n");
	//}
	
	//char	buffer[1000];
	//ssize_t	bytesRead;
	//while ((bytesRead = read(_sockfd, buffer, sizeof(buffer) - 1)) > 0)
	//{
	//	buffer[bytesRead] = '\0';
	//	std::cout << "Received response: " << buffer;
	//}
	//if (bytesRead < 0)
	//	throw std::runtime_error("Error reading response\n");
	//buffer[999] = '\0';
	//std::cout << "Read by client: " << buffer << "\n";




				//else
			//{
			//	/* TESTING OLD CLIENT FUNCTION FROM HERE */
			//	//int		clientSockfd = events[i].data.fd; //events[i].data.fd
			//	//socklen_t	clientAddrlen = client.getAddrlen();
			//	//struct sockaddr_in	clientSockaddr = client.getSockaddr();
				
			//	int clientSockfd = client.getSockfd();
			//	if ((connect(clientSockfd, (struct sockaddr *)&serverSockaddr, serverAddrlen)) < 0)
			//	{
			//		close(clientSockfd);
			//		throw std::runtime_error("Error connecting to server from client\n");
			//	}
			//	std::cout << "Client connected successfully to port\n";

			//	std::string message = "GET / HTTP/1.1\r\nHost: " + client.getHost() + "\r\nConnection: close\r\n";
			//	if (send(clientSockfd, message.c_str(), message.size(),0) < 0)
			//	{
			//		close(clientSockfd);
			//		throw std::runtime_error("Error sending message from client\n");
			//	}
				
			//	char	buffer[1000];
			//	ssize_t	bytesRead;
			//	while ((bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1)) > 0)
			//	{
			//		buffer[bytesRead] = '\0';
			//		std::cout << "Received response: " << buffer;
			//	}
			//	if (bytesRead < 0)
			//		throw std::runtime_error("Error reading response\n");
			//	buffer[999] = '\0';
			//	std::cout << "Read by client: " << buffer << "\n";
			//	closeDelete(clientSockfd, _epfd);
			//	std::cout << "\nClosed client socket and deleted from Epoll\n";
				
			//}
			
			
			/* KEEP GETTING ERROR READING FROM CLIENT THROW WITH BELOW CODE */
			//else
			//{
			//	// handle incoming data from client connection
			//	int			clientSockfd = events[i].data.fd;
			//	char		buffer[1024];
			//	ssize_t		bytesRead; // = 0;
			//	std::string	request;

			//	//while ((bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1)) > 0)
			//	//{
			//	//	buffer[bytesRead] = '\0';
			//	//	request.append(buffer);
			//	//}
			//	bytesRead = read(clientSockfd, buffer, sizeof(buffer) - 1);
			//	if (bytesRead > 0)
			//	{
			//		buffer[bytesRead] = '\0';
			//		std::cout << "Received request: " << request << "\n";
			//		std::string response = generateHttpResponse("HELLO WORLLLLLD");
			//		send(clientSockfd, response.c_str(), response.size(), 0);
			//	}
			//	//if (!request.empty())
			//		//std::cout << "Received request: " << request << "\n";
			//	//else
			//	//	std::cout << "Request empty\n";
				
			//	if (bytesRead < 0)
			//	{
			//		closeDelete(clientSockfd, _epfd);
			//		throw std::runtime_error("Error reading from client\n");
			//	}
			//	closeDelete(clientSockfd, _epfd);
			//	std::cout << "\nClosed client socket and deleted from Epoll\n";
			//}
		//}


							//std::string response = generateHttpResponse("HELLO WORLD");
					//if (response.empty())
					//{
					//	std::cerr << "Failed to create HttpResponse\n";
					//	continue ;
					//}
					//if (send(serverSockfd, response.c_str(), response.size(), 0) < 0)
					//{
					//	std::cerr << "Failed to send response from server\n";
					//	continue ;
					//}



#########################################################################