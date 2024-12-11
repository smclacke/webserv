/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   serverData.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/04 14:46:58 by smclacke      #+#    #+#                 */
/*   Updated: 2024/12/11 14:14:03 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/web.hpp"
#include "../../include/epoll.hpp"

void s_serverData::addClient(int sock, struct sockaddr_in &addr, int len, Epoll &epoll)
{
	auto newClient = t_clients(epoll, *_server);
	newClient._fd = sock;
	newClient._addr = addr;
	newClient._addLen = len;
	newClient._clientState = clientState::BEGIN;
	newClient._clientTime[sock] = std::chrono::steady_clock::now();
	newClient._connectionClose = false;
	newClient._write_offset = 0;
	newClient._bytesWritten = 0;
	newClient._readingFile = false;

	_clients.push_back(newClient);
}

void s_serverData::removeClient(t_clients &client)
{
	auto it = std::find_if(_clients.begin(), _clients.end(), [&client](t_clients &c)
						   { return &c == &client; });

	if (it != _clients.end())
		_clients.erase(it);
	else
		std::cerr << "Error: Client not found in _clients deque\n";
}

s_clients::s_clients(Epoll &epoll, Server &server)
	: _fd(-1), _addLen(0), _clientState(clientState::BEGIN), _connectionClose(false),
	  http(std::make_shared<httpHandler>(server, epoll)), _write_offset(0), _bytesWritten(0), _readingFile(false)
{
	// Initialize other members if necessary
}
