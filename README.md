# Codam-42 Webserv 🌐

**Project status: finished ✅**

## About the project 📝

The goal of webserv is to write a fully custom webserver in CPP. 
The webserver should be able to:
- Parse a configuration file
- Allow for listening on multiple ports
- Handle multiple clients while leaving the connection open
- Process an HTTP request with GET, POST or DELETE
- Give an appropriate HTTP response based on the request
- Provide webpages to display the functioning of the webserver

## Project parts 🔧

The project can be divided up in the following parts:
- Config file parsing
- Setting up the Epoll() system
- Handling EPOLLIN, EPOLLOUT and error related events
- Parsing the HTTP request headers
- Parsing the HTTP request body, based on the parameters of the request headers
- Generating the right Response
- Executing a CGI script without holding up I/O
- Having efficient Read/Write operations

## Writing a big project in C++

Because we wanted to work on different aspects at the same time and to organize functions and data easily we created a class for each aspect of the project. We ended up with the following classes:

- webserv: The starter and ender of all processes. This is the overarching class that calls all other classes to run the server.
- server: each server defined in the config file gets its own instance of Server.
- socket: each server gets its own websocket that will be connected to the epoll().
- epoll: the class that handles all the traffic and the one instance of epoll().
- httpHandler: this class is responsible for reading the HTTP request and generating the response.

## 3 main challenges

#### Understanding HTTP Requests

[Information about HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers)

##### Reading the request 

HTTP requests are at the core of this project. An HTTP request has specific formatting and depending on the information provided by the request, the right response should be generated. Since we are only implementing GET, POST and DELETE requests, here are 3 examples:

*Simple GET request to get index.html*

>GET /index.html HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0
Accept: text/html
Connection: keep-alive

*Simple DELETE request to delete file.txt*
>DELETE /file.txt HTTP/1.1
Host: localhost:8080
Connection: keep-alive

*A POST request with Content-Type multipart/form-data*
>POST /upload.txt HTTP/1.1
Host: localhost:8080
Content-Type: multipart/form-data; boundary=---------------------------974767299852498929531115
Content-Length: 554
-----------------------------974767299852498929531115
Content-Disposition: form-data; name="username"
john_doe
-----------------------------974767299852498929531115
Content-Disposition: form-data; name="email"
john@example.com
-----------------------------974767299852498929531115
Content-Disposition: form-data; name="file"; filename="hello.txt"
Content-Type: text/plain
Hello World!
-----------------------------974767299852498929531115--


The headers are built out of:
>METHOD URI HTTPversion

>Headers (can be any amount of headers)

>\r\n\r\n

>Body (optional)

GET and DELETE requests are the most simple to implement since they generally don't have a body. If they do, in most cases a 400 Bad Request response should be returned.

Due to POST requests having a body, the headers of the request are very important. These determine how the body is parsed and what should be done with it.
For parsing of the body, one of the headers we chose to implement is chunked encoding:
When the Content-Encoding header is set to chunked, the body is sent in small chunks.

For what to do with the body, not only the URI is important but also the Content-Type.
We chose to implement a few Content-Types:
- No Content
- multipart/form-data
- application/*
- text/plain

These Content-Type settings cover a wide variety of body input for our simple webserver.

##### Generating the response

HTTP responses are formatted the following way:

>HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 13
\r\n
Hello, World!

With:
>HTTP version + Status Code + Status Message

>Headers (any amount possible)

>\r\n\

>Body (optional)


A request should always have a response. In case something is wrong with even the first line of the request, the server should still reply. In the case of a bad first line, the server would respond with a 400 Bad Request code. We put all of the response codes in httpConstants.hpp for easier and more readable programming.

HTTP status codes are defined in categories:
- 100 range usually means the server is processing something.
- 200 range indicates success, usually 200 OK.
- 300 range has to do with redirects, which means the location referred to by the URI is no longer in use and an alternative is usually provided by the response headers.
- 400 range is some error with processing the request.
- 500 range is usually something to do with an error on the server side, i.e., the server couldn't generate a file, something requested is not implemented, or some other internal error.

We did not end up using all HTTP codes in our server, but we implemented a large variety with the goal to provide the most information-specific output.

#### Handling connection with efficient writing/reading for the socket

For this project, we were allowed to choose between select(), poll(), and epoll() for handling a single websocket.

*Since our systems run on Ubuntu and epoll() is the fastest and most modern option, we went with epoll().*

What are the challenges of epoll()?
First up is understanding the wide range of settings you need to implement, a good understanding of how sockets work, and what information epoll() needs to start listening on the right ports and addresses.

After getting a simple websocket setup, epoll.wait will generate EPOLL events. If everything goes right, these events are either EPOLLIN, EPOLLOUT, or EPOLLHUP. EPOLLHUP usually means the connection closed and is not very interesting. EPOLLIN means there is a client that wants to connect and send information to the server. EPOLLOUT means we can start sending information to the client.

One important thing to implement is that all clients get their attention. If one client is sending 10,000 bytes of information, it should not impact a client making a very simple request. This is why we read the information that each client sends the server in small READ_BUFFER_SIZE of 100 bytes. We do the same thing for writing. This way, while processing a big request, we can simultaneously read and then send to a client making a small request.

#### Implementing a non-blocking CGI

What is CGI:
Common Gateway Interface protocols. This allows clients to make a request to the server where the server will execute a program (inside a child process). This allows for the execution of .cgi or .php scripts in the case of our webserv. These scripts need to be able to take input from, for example, a POST HTTP request body and generate the right output.

**The challenge**

Firstly, the script needs to run inside a child process, ensuring that you can safely execute the script without affecting your main process. However, while the script is running, reading and writing to other clients must continue. This means the script cannot block your program's I/O operations. How do you avoid blocking I/O operations? You already have a handler for this: epoll() can manage multiple I/O operations. To add your input and output from your CGI script to epoll, you need to pipe() the IN and OUT of the child process executing the script. Additionally, to keep everything running smoothly, you must check for these pipe() file descriptors inside your epoll() loop and ensure the script doesn't run indefinitely by implementing a script timeout check. Incorporating CGI with the epoll() loop was quite a challenge and required some rewriting, something we should have probably considered earlier.