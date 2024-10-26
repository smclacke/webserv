
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
		sa_family_t    sin_family; /* address family: AF_INET */
		in_port_t      sin_port;   /* port in network byte order */
		struct in_addr sin_addr;   /* internet address */
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



