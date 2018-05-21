#include <string>

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
}
