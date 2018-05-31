#include <string>
#include <iostream>

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "inet/Socket.hpp"

namespace inet
{
	Socket::Socket(int f, int t, int p) : family(f), type(t), protocol(p)
	{
		this->socket = ::socket(f, t, p);

		if(this->socket == -1)
		{
			throw "Bad socket";
		}
	}

	Socket::Socket(int capture, int f, int t, int p) : socket(capture), family(f), type(t), protocol(p) {}

	Socket::~Socket(void)
	{
		int result = close(this->socket);

		if(result == -1)
		{
			std::cout << std::string("Socket::~Socket() Error closing socket: ") + std::to_string(errno) << std::endl;
		}
	}

	void Socket::listen(void)
	{
		int result = ::listen(this->socket, SOMAXCONN);
		if(result == -1)
		{
			throw std::string("Socket::listen failed: ") + std::to_string(errno);
		}
	}

	Socket::operator int() const
	{
		return this->socket;
	}
}
