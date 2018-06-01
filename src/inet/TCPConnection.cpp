
#include "sys/socket.h"
#include "inet/TCPConnection.hpp"

namespace inet
{
	TCPConnection::TCPConnection(void) : IPConnection(SOCK_STREAM, 0) {}
	TCPConnection::TCPConnection(int captureRawSocket, sockaddr_in& captureAddr) : IPConnection(captureRawSocket, SOCK_STREAM, 0, captureAddr) {}

	bool TCPConnection::send(void* data) const
	{
		if(data == nullptr)
		{
			return false;
		}
		return true;
	}
}
