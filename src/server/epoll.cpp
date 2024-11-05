/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   epoll.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/22 15:02:59 by smclacke      #+#    #+#                 */
/*   Updated: 2024/11/05 21:46:30 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"

/* constructors */

Epoll::Epoll() {}


Epoll::Epoll(const Epoll &copy)
{
	*this = copy;
}

Epoll &Epoll::operator=(const Epoll &epoll)
{
	if (this != &epoll)
	{
		this->_epfd = epoll._epfd;
		this->_op = epoll._op;
		this->_numEvents = epoll._numEvents;
		// timeout
	}
	return *this;
}

Epoll::~Epoll() { close(_epfd); }


/* methods */

static std::string generateHttpResponse(const std::string &message)
{
	size_t	contentLength = message.size();
	std::ostringstream	response;
	response	<< "HTPP/1.1 200 OK\r\n"
				<< "Content-Type: text/plain\r\n"
				<< "Content-Length: " << contentLength << "\r\n"
				<< "Connection : close\r\n"
				<< "\r\n"
				<< message;
	
	return response.str();
}

void		Epoll::initEpoll()
{
	_epfd = epoll_create1(0); // can't use epoll_Create1 but for now i am mwhaha
	if (_epfd < 0)
		throw std::runtime_error("Error creating Epoll instance\n");
	std::cout << "Successfully created Epoll instance\n";
}

static struct epoll_event addSocketEpoll(int sockfd, int epfd)
{
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = sockfd;
	
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
	{
		close(sockfd);
		throw std::runtime_error("Error adding server socket to epoll\n");
	}
	std::cout << "Successfully added server socket to epoll\n";
	return event;
}

static void		addConnectionEpoll(int connection, int epfd, struct epoll_event event)
{
	event.events = EPOLLIN;
	event.data.fd = connection;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, connection, &event) < 0)
	{
		close(connection);
		throw std::runtime_error("Error adding connection to epoll\n");
	}
	std::cout << "Successfully added connection to epoll\n";
}

static	void	setNonBlocking(int connection)
{
	int	flag = fcntl(connection, F_GETFL, 0);
	fcntl(connection, F_SETFL, flag | O_NONBLOCK);
}

static void		closeDelete(int fd, int epfd)
{
	close(fd);
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void		Epoll::monitor(Socket &server, Socket &client)
{
	int		serverSockfd = server.getSockfd();
	socklen_t	serverAddrlen = server.getAddrlen();
	struct sockaddr_in	serverAddr = server.getSockaddr();
	
	int		clientSockfd = client.getSockfd();
	socklen_t	clientAddrlen = client.getAddrlen();
	struct sockaddr_in	clientAddr = client.getSockaddr();

	// add server socket to epoll
	struct epoll_event event = addSocketEpoll(serverSockfd, _epfd);

	while (true)
	{
		struct epoll_event events[10];
		int numEvents = epoll_wait(_epfd, events, 10, -1); // wait for events

		for (int i = 0; i < numEvents; ++i)
		{
			if (events[i].data.fd == serverSockfd)
			{
				// accept new connection
				serverAddrlen = server.getAddrlen();
				serverAddr = server.getSockaddr();

				int newConnection = accept(serverSockfd, (struct sockaddr *)&serverAddr, &serverAddrlen);
				if (newConnection < 0)
				{
					std::cerr << "Error accepting new connection\n";
					continue ;
				}
				else 
				{
					server.setNewConnection(newConnection);
					std::cout << "Successfully made connection\n";
					setNonBlocking(newConnection);
					addConnectionEpoll(newConnection, _epfd, event);

					/* TEST STUFF*/
					// read from connection
					char buffer[1000];
					std::string request;
					ssize_t bytesRead;
					while ((bytesRead = read(newConnection, buffer, sizeof(buffer) - 1)) > 0)
					{
						buffer[bytesRead] = '\0';
						request += buffer;
						if (request.find("\r\n\r\n") != std::string::npos)
							break ; // end of request
					}
					if (bytesRead < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
						throw std::runtime_error("Error reading from new connection\n");
					if (!request.empty())
					{
						std::cout << "Received request: " << request << "\n";
						std::string response = generateHttpResponse("HELLO WORLLLLLD");
						if (send(newConnection, response.c_str(), response.size(), 0) < 0)
							throw std::runtime_error("Error sending response\n");
						std::cout << "Successfully sent response \"HELLO WORLLLLLD\"\n";
					}
					else
						std::cout << "Received empty request\n";

					closeDelete(newConnection, _epfd);
				}
			}
			
			/* TESTING OLD CLIENT FUNCTION FROM HERE */
			// attempt to connect
			else
			{
				//int	sockfd = events[i].data.fd;
				int	sockfd = client.getSockfd();
				socklen_t	addlen = client.getAddrlen();
				struct sockaddr_in sockadd = client.getSockaddr();
				
				if ((connect(sockfd, (struct sockaddr *)&sockadd, addlen)) < 0)
				{
					close(sockfd);
					throw std::runtime_error("Error connecting to server from client\n");
				}
				std::cout << "Client connected successfully to port\n";

				std::string message = "GET / HTTP/1.1\r\nHost: " + client.getHost() + "\r\nConnection: close\r\n";
				if (send(sockfd, message.c_str(), message.size(),0) < 0)
				{
					close(sockfd);
					throw std::runtime_error("Error sending message from client\n");
				}
				
				char	buffer[1000];
				ssize_t	bytesRead;
				while ((bytesRead = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
				{
					buffer[bytesRead] = '\0';
					std::cout << "Received response: " << buffer;
				}
				if (bytesRead < 0)
					throw std::runtime_error("Error reading response\n");
				buffer[999] = '\0';
				std::cout << "Read by client: " << buffer << "\n";
				closeDelete(sockfd, _epfd);
				std::cout << "\nClosed client socket and deleted from Epoll\n";
				
			}
			
			
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
		}
	}
	closeDelete(serverSockfd, _epfd);
	std::cout << "\nClosed server socket and deleted from Epoll\n";
}
