
#include "sys/socket.h"
#include "inet/TCPConnection.hpp"

namespace inet
{
	TCPConnection::TCPConnection(void) : IPConnection(SOCK_STREAM, 0) {}

	TCPConnection::TCPConnection(int captureRawSocket, IPConnection& parentConnection, sockaddr_in& destAddr) : IPConnection(captureRawSocket, SOCK_STREAM, 0, parentConnection, destAddr) {}

	bool TCPConnection::send(void* data) const
	{
		if(data == nullptr)
		{
			return false;
		}
		return true;
	}
}
