/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/21 19:20:28 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include "../include/web.hpp"

int	main()
{
	// create socket (IPv4, TCP)
	int	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cout << "failed to create socket\n";
		exit (EXIT_FAILURE);
	}
	std::cout << "successfully created socket\n";


	// listen to port 9999 on any address
	sockaddr_in	sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(9999); // htons to convert a number to network byte order

	if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
	{
		std::cout << "failed to bind to port 9999\n";
		exit (EXIT_FAILURE);
	}
	std::cout << "binding to port 9999 successful\n";


	// start listening, hold at most 10 connections in the queue
	if (listen(sockfd, 10) < 0)
	{
		std::cout << "failed to listen on socket\n";
		exit (EXIT_FAILURE);
	}
	std::cout << "listening successfully\n";


	// grab a connection from the queue
	auto 	addrlen = sizeof(sockaddr);
	int		connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	if (connection < 0)
	{
		std::cout << "failed to grab connection\n";
		exit (EXIT_FAILURE);
	}
	std::cout << "successfully made connection\n";
	
	// read fromm the connection
	char	buffer[100];
	size_t	bytesRead = read(connection, buffer, 100); // unused bytesRead error?
	std::cout << "read message from connection: " << buffer;

	// send message to the connection
	std::string	response = "nice chatting with you connection :) \n";
	send(connection, response.c_str(), response.size(), 0);

	// close the connections
	close(connection);
	std::cout << "successfully closed connection\n";

	close(sockfd);	
	std::cout << "successfully closed sockfd\n";

}
