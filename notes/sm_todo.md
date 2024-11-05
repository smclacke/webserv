

**TODOTODOTODO**


1) check with julius how we're handling try/catch so i can just do it all now correctly (will main catch any throw?)
-- for multiple, when server has been through requests etc, server sockets will be closed
		and deleted from epoll to make space for the next server

2) stuff in main - figure out with julius (multiple server monitoring etc //server[1]...)

3) close protection + double check other cleaning

4) vector of connections, how what where why



-----------------------------------------------------------------

**WHEREIAT**

- got two client sort of versions in epoll loop, testing extra server stuff

- not totally erroring, curl give HTTP 0.9 not allowed
- browser no message page not found :( locahost refused to connect / not allowed something
