Steps to Integrate epoll with httpHandler:
1. Initialize httpHandler:
Create an instance of httpHandler for each client connection or reuse a single instance if your design allows it.
2. Read and Parse Requests:
Use epoll to monitor file descriptors for incoming data.
When data is available, read it into a buffer and append it to a std::stringstream or similar structure.
Once the headers are fully received (detected by \r\n\r\n), use httpHandler::parseRequest to parse the request.
3. Handle Request and Generate Response:
After parsing the request, call httpHandler::generateResponse to process the request and prepare a response.
The response can be written back to the client using the file descriptor monitored by epoll.
4. Manage Connection State:
Use the keepalive flag in the s_response struct to determine if the connection should be kept open or closed after the response is sent.

Scenarios for Parsing the Request:
1. Content-Length:
If the Content-Length header is present, it specifies the exact number of bytes in the body. You should continue reading until you have read this many bytes.
2. Transfer-Encoding: chunked:
If the Transfer-Encoding: chunked header is present, the body is sent in chunks. You need to read and process each chunk until you encounter a zero-length chunk (0\r\n\r\n).
Multipart/form-data:
If the request is multipart/form-data, you need to read until the boundary string is found, which marks the end of the multipart data. The boundary is specified in the Content-Type header.
Implementation Steps:
1. Parse Headers:
Read the headers to determine which scenario applies. Extract Content-Length, Transfer-Encoding, and Content-Type headers as needed.
2. Read the Body:
Based on the headers, decide how to read the body:
Content-Length: Read the specified number of bytes.
Chunked: Read chunks until a zero-length chunk is encountered.
Multipart: Read until the boundary string is found.
3. Generate Response:
Once the entire request is read and parsed, call generateResponse to process the request and prepare a response.