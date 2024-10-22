
#########################################################################

**GIT**


		<  git push -u origin <branch>  >

-u = --set-upstream


#########################################################################


**CHECK PORTS AND KILL IF LEFT OPEN AND LISTENING**

lsof -i :9999 - check open ports

kill <PID> - get PID from lsof info
kill -9 <PID> - alt if kill alone doesnt work


#########################################################################


**SOCKET_SERVER**


***SOCKET***

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


***BIND***

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


***LISTEN***

marks a socket as passive (socket will be used to accept connections)

< int	listen(int sockfd, int backlog); >

	returns -1 on error, success 0

	sockfd - fd of socket

	backlog - maximum number of connections that will be queued before connections start being refused


***ACCEPT*** 

extracts an element from a queue of connections (the queue created by listen) for a socket 

< int	accept(int sockfd, sockaddr *addr, socklen_t *addrlen); >

	returns -1 on error, returns fd for connection on success

	arguments are similar to bind() but addrlen is now value-result argument
		it expects a pointer to an int that will be the size of addr
		after the function is executed, the int refered by addrlen will be set to the size of the peer address



