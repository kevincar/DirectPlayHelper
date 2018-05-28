
#include "sys/socket.h"
#include "inet/TCPConnection.hpp"

namespace inet
{
	TCPConnection::TCPConnection(void) : IPConnection(SOCK_STREAM, 0) {}

	bool TCPConnection::send(void* data) const
	{
		if(data == nullptr)
		{
			return false;
		}
		return true;
	}
}
