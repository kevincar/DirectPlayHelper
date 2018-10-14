
#include <iostream>
#include "sys/socket.h"
#include "inet/UDPConnection.hpp"

namespace inet
{
	UDPConnection::UDPConnection() : IPConnection(SOCK_DGRAM, 0) {}

	int UDPConnection::sendTo(char const* data, unsigned int const data_len, ServiceAddress const& addr) const
	{
		long result = ::sendto(*this, data, data_len, 0, addr, sizeof(sockaddr_in));
		return static_cast<int>(result);
	}

	int UDPConnection::recvFrom(char* buffer, unsigned int const buffer_len, ServiceAddress& addr) const
	{
		unsigned int addr_len = sizeof(sockaddr);
		long result = ::recvfrom(*this, buffer, buffer_len, 0, addr, &addr_len);
		return static_cast<int>(result);
	}
}
