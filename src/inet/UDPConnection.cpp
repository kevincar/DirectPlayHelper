
#include <iostream>
#include "sys/socket.h"
#include "inet/UDPConnection.hpp"

namespace inet
{
	UDPConnection::UDPConnection() : IPConnection(SOCK_DGRAM, 0) {}

	bool UDPConnection::send(void* data) const
	{
		std::cout << "Not yet implemented";
		if(data == nullptr)
		{
			return false;
		}
		return true;
	}
}
