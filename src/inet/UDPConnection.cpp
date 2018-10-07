
#include <iostream>
#include "sys/socket.h"
#include "inet/UDPConnection.hpp"

namespace inet
{
	UDPConnection::UDPConnection() : IPConnection(SOCK_DGRAM, 0) {}

	int UDPConnection::sendTo(void const* data, unsigned int const data_len, ServiceAddress const& addr) const
	{
		long result = ::sendto(*this, data, data_len, 0, addr, sizeof(sockaddr_in));
		return static_cast<int>(result);
	}
}
