/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   serverData.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/04 14:46:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/04 14:49:06 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

void	s_serverData::addClient(int sock, struct sockaddr_in &addr, int len)
{
	static int clientId = 0;

	t_clients	newClient;

	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;
	newClient._clientState = clientState::BEGIN;
	newClient._clientTime[sock] = std::chrono::steady_clock::now();
	newClient._connectionClose = false;
	newClient._write_offset = 0;
	newClient._bytesWritten = 0;
	newClient._clientId = clientId++;
	
	_clients.push_back(newClient);

	std::cout << "New client connected with ID: " << newClient._clientId << " from " << inet_ntoa(newClient._addr.sin_addr) << "\n";
}

void		s_serverData::removeClient(t_clients &client)
{
	auto it = std::find_if(_clients.begin(), _clients.end(), [&client](const t_clients &c)
	{
		return &c == &client;	
	});

	if (it != _clients.end())
	{
		_clients.erase(it);
		std::cout << "Client successfully removed from deque\n";
	}
	else
		std::cerr << "Error: Client not found in _clients deque\n";
}
