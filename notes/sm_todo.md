

**TODOTODOTODO**

1) 
	cleanUp function? necessary, anything left behind after success? (i dont think so)

2) 
	add descriptions/briefs

5) 
	error handling + closing + handling of fds/sockets etc
	run through full code to check im not missing a close etc

6) 
	handling CHUNK information streams..
	the current chunk will have size of next chunk etc.. need to chunkHandle

7) 
	adding files to epoll (still can't make sense of this...)



**13)** 
	CHECK - why am i making connections when i have only typed the address in
		and not entered in browser?
8) 
	TEST the current monitoring - something that will take a few loops to be read etc..

16) CHECK - server/client for loops in monitoring logic sound?


**JULIUS**
11) 
	ASK julius - if server - server socket creation fails in openServerSocket, just give error
	message and continuing creating the others if there are any? + if only one then what? 

14) 
	CHECK - JULIUS: openserver socket erroring handling

14) CHECK - not handling invalid fd in recv... no one else does?
	e only call recv() if fd (epoll events) == client.fd so that already verifies that client.fd is valid
