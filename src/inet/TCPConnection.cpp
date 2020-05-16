
#include "inet/TCPConnection.hpp"

namespace inet
{
	TCPConnection::TCPConnection(void) : IPConnection(SOCK_STREAM, 0) {}

	TCPConnection::TCPConnection(int captureRawSocket, IPConnection const& parentConnection, sockaddr_in& destAddr) : IPConnection(captureRawSocket, SOCK_STREAM, 0, parentConnection, destAddr) {}
}
