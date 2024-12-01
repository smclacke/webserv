

**TODOTODOTODO**

1) 
	cleanUp function for monitoring fail or end - anything that should be deleted from epoll...
		not in destructors (epoll destructor delete anything that was added to the epoll here?)

2) 
	add some descriptions/briefs

3) 
	stuff im not sure about:
		- when close connections: once whole request/response received and handled: fix this in code

5) 
	everywhere good error handling + closing + handling of fds/sockets etc

6) 
	handling CHUNK information streams..
	the current chunk will have size of next chunk etc.. need to chunkHandle

7) 
	adding files to epoll

8) 
	TEST the current monitoring - something that will take a few loops to be read etc..

9) 
	some exit / throws not necessary
	some things should give error, be handled and server continues

10) 
	im reusing the same client info... need to make sure im creating new instances each time

11) 
	ASK julius - if server - server socket creation fails in openServerSocket, just give error
	message and continuing creating the others if there are any? + if only one then what? 

12) 
	CHECK - close connection after each read and write or only after the write?

13) 
	CHECK - why am i making connections when i have only typed the address in
		and not entered in browser?