#include <string>

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

	Socket::~Socket(void) noexcept(false)
	{
		int result = close(this->socket);

		if(result != 0)
		{
			throw std::string("Error closing socket: ") + std::to_string(errno);
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
