
#include "sys/socket.h"
#include "inet/UDPConnection.hpp"

namespace inet
{
	UDPConnection::UDPConnection() : IPConnection(SOCK_DGRAM, 0) {}
}
