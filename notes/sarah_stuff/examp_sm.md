


Client State Management:

You are updating client._clientState to clientState::READING and clientState::WRITING, but once you detect that the client has completed reading or writing (e.g., the full HTTP request or response is processed), you reset it to clientState::READY. This state flow looks good, but there are some edge cases where the state machine could be more robust, such as:
Handling unexpected disconnections or incomplete requests/responses.
Managing a transition from READY back to READING or WRITING if more data is expected after an initial request/response.
Buffer Size (READ_BUFFER_SIZE / WRITE_BUFFER_SIZE):

You're using READ_BUFFER_SIZE and WRITE_BUFFER_SIZE to control the size of each read and write operation. However, these buffers should be fine-tuned for your use case:
If you expect very large requests or responses, consider dynamically adjusting the buffer size based on the needs of the client (e.g., if a request is too large to fit into a single buffer).
Similarly, if the response is large and might take multiple writes, you correctly handle the write offset, but ensure the buffer size is large enough to handle the average message size.
Error Handling:

Your error handling looks solid (e.g., handling EAGAIN for non-blocking I/O). However, ensure you're handling all possible error conditions:
recv() failure with -1: You currently check EAGAIN and EWOULDBLOCK, but other error codes like ECONNRESET (connection reset) could also occur, so make sure you're checking for these cases and handling them appropriately.
Graceful client disconnection: You handle bytesRead == 0 (client disconnect), but make sure that any client disconnection (whether intentional or due to network issues) is cleanly handled, especially with regards to closing file descriptors.
Response Generation Optimization:

In the handleWrite() function, you're generating a response (HTTP status) each time the buffer is written to the client. This may be fine for simple cases, but ensure that the response generation (like the Content-Length header or any dynamic content) isn't unnecessarily repeated for every write. It would be more efficient to prepare the full response once and send it in chunks as needed.
Consider buffering responses in memory (e.g., building the entire response in memory and sending it in chunks), especially if responses are large or generated dynamically.
Connection Management:

It's great that you're considering whether to keep the connection alive or close it after each request (client._connectionClose). This can be determined based on the HTTP request headers (e.g., Connection: keep-alive or Connection: close), and you already plan to handle that in the @todo section.
You should ensure that the logic for closing the connection is implemented after finishing reading and writing the entire response. Additionally, you might want to handle graceful shutdown or handling of stale client connections.
Optimizing recv() & send() Loops:

Consider using a more efficient read/write loop, especially for large data. Right now, you're reading and writing in fixed-size chunks (READ_BUFFER_SIZE and WRITE_BUFFER_SIZE). Depending on the protocol, you might want to implement a more flexible system for dynamically allocating buffer sizes or breaking large requests/responses into smaller chunks.
Dynamic buffer expansion: If the buffer is smaller than the request or response, you might need to dynamically allocate more memory for the request/response.
Timeout Handling:

If you haven't already, consider implementing timeout handling for long or stalled connections, which is important for managing resources (e.g., with epoll_wait()).
Set a timeout for how long you wait for data to be read from a client, and close the connection if no data is received within that time.











/** @todo add bool for close or keep connection alive depneding on the header */
void		Epoll::handleRead(t_serverData &server, t_clients &client)
{
	char			buffer[READ_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	ssize_t 		bytesRead = recv(client._fd, buffer, sizeof(buffer) - 1, 0);
	std::string		request; // http request

	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			std::cout << "no data available right now\n";	
			return ;
		}
		// recv failed
		handleClose(server, client);
		throw std::runtime_error("Reading from client connection failed\n");
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected\n";
		handleClose(server, client);
		return ;
	}
	
	// read protocol
	
	// use client state READING while still busy

	buffer[bytesRead] = '\0';
	request += buffer;
	std::cout << "Server received " << request << " from client " << client._fd << " \n";
	client._clientState = clientState::READY;
	// http request handling here
	// bool _connectionClose

}

/** @todo add bool for close or keep connection alive depneding on the header
 * @todo ensure generaterequest is not being called everytime for the same client when just filling buffer
 */
void		Epoll::handleWrite(t_serverData &server, t_clients &client)
{
	(void) server; // why am i not using server?
	if (client._response.empty()) // || if client state begin/ready, then generate
	{
		// if not started writing response, generate 
		//const char	response[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
		//std::string	response1 = generateHttpResponse("this message from write");
	
		//client._response = generateHttpResponse("this message from handleWrite()");
		client._response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	}

	// write protocol
	while (client._bytesWritten < WRITE_BUFFER_SIZE)
	{
		client._bytesWritten = send(client._fd, client._response.c_str() + client._write_offset, strlen(client._response.c_str()) - client._write_offset, 0);
		if (client._bytesWritten == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				client._clientState = clientState::WRITING;
				client._connectionClose = false;
				return ; // no space in socket's send buffer, wait for more space
			}
			std::cerr << "Write to client failed\n";
			//handleClose(server, client);
			client._connectionClose = true;
			client._clientState = clientState::ERROR;
			return ;
		}
		
		client._clientState = clientState::WRITING;
		client._write_offset += client._bytesWritten;
		if (client._write_offset == strlen(client._response.c_str()))
		{
			client._write_offset = 0;
			std::cout << "Client " << client._fd << " sent message to server: " << client._response << "\n\n\n";
			client._connectionClose = false; // true? we done
			client._clientState = clientState::READY;
			return ;
		}
		client._connectionClose = false;
	}
}


        // Append the read data to the client's requestData
        client.requestData.append(buffer, bytesRead);

        // Check if the request is complete
        if (isRequestComplete(client.requestData)) {
            client.doneReading = true;
            break;
        }
    }

    if (client.doneReading) {
        // Modify epoll to watch for writable events
        epoll_event event;
        event.data.fd = client.fd;
        event.events = EPOLLOUT; // Change to write
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, client.fd, &event) == -1) {
            perror("epoll_ctl: modify to EPOLLOUT");
            close(client.fd);
        }


// OLDER


/* EXAMPLE */

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

/* EXMAPLE TWO */

//char		buffer[1024];
//				int			bytesRead = 0;
//				std::string	request;
				
//				while (read(events[i].data.fd, buffer, sizeof(buffer) - 1) < 0)
//				{
//					buffer[bytesRead] = '\0';
//					request += buffer;
//					if (request.find("\r\n\r\n") != std::string::npos)
//						break ; // end of HTTP request
//				}
//				if (bytesRead <= 0)
//				{
//					close (events[i].data.fd);
//					std::cout << "Client disconnected\n";
//				}
//				if (!request.empty())
//				{
//					std::cout << "Server received request: " << request << "\n";
					
//					// send HTTP response
//					std::string	response = generateHttpResponse("here, have a response");

//					if (send(events[i].data.fd, response.c_str(), response.size(), 0) < 0)
//						std::cerr << "Error sending response to client\n";
//				}
//				else
//					std::cout << "received empty request\n";
//			}
