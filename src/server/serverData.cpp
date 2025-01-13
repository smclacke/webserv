/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   serverData.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/12/04 14:46:58 by smclacke      #+#    #+#                 */
/*   Updated: 2025/01/10 14:45:56 by jde-baai      ########   odam.nl         */
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
	newClient._readingFile = false;

	_clients.emplace_back(std::move(newClient));
}

void s_serverData::removeClient(t_clients &client)
{
	auto it = std::remove_if(_clients.begin(), _clients.end(), [&client](t_clients &c)
							 { return &c == &client; });

	if (it != _clients.end())
		_clients.erase(it, _clients.end());
	else
		std::cerr << "removeClient(): error: Client not found in _clients vector\n";
}

s_clients::s_clients(Epoll &epoll, Server &server)
	: _fd(-1), _addr(), _addLen(0), _clientState(clientState::BEGIN), _clientTime(), _connectionClose(false),
	  http(std::make_unique<httpHandler>(server, epoll)), cgi(), _responseClient(),
	  _write_offset(0), _readingFile(false), bytesReadtotal(0)
{
}

// Implement move constructor
s_clients::s_clients(s_clients &&other) noexcept
	: _fd(other._fd), _addr(other._addr), _addLen(other._addLen), _clientState(other._clientState),
	  _clientTime(std::move(other._clientTime)), _connectionClose(other._connectionClose),
	  http(std::move(other.http)), cgi(other.cgi),
	  _responseClient(std::move(other._responseClient)), _write_offset(other._write_offset),
	  _readingFile(other._readingFile), bytesReadtotal(other.bytesReadtotal)
{
	other._fd = -1;
}

// Implement move assignment operator
s_clients &s_clients::operator=(s_clients &&other) noexcept
{
	if (this != &other)
	{
		_fd = other._fd;
		_addr = other._addr;
		_addLen = other._addLen;
		_clientState = other._clientState;
		_clientTime = std::move(other._clientTime);
		_connectionClose = other._connectionClose;
		http = std::move(other.http);
		cgi = other.cgi; // No std::move needed
		_responseClient = std::move(other._responseClient);
		_write_offset = other._write_offset;
		_readingFile = other._readingFile;
		bytesReadtotal = other.bytesReadtotal;

		other._fd = -1;
	}
	return *this;
}