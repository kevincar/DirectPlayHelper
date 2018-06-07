
#include "sys/socket.h"
#include "inet/TCPConnection.hpp"

namespace inet
{
	TCPConnection::TCPConnection(void) : IPConnection(SOCK_STREAM, 0) {}

	TCPConnection::TCPConnection(int captureRawSocket, IPConnection const& parentConnection, sockaddr_in& destAddr) : IPConnection(captureRawSocket, SOCK_STREAM, 0, parentConnection, destAddr) {}

	void TCPConnection::connect(std::unique_ptr<ServiceAddress>& destAddress)
	{
		// Set the destAddress
		{
			std::lock_guard<std::mutex> lock {this->destAddr_mutex};
			this->destAddress = std::move(destAddress);

			std:: lock_guard<std::mutex> sock_lock {this->socket};
			//int result = ::connect(*this->socket.get(), )
		}
	}

	bool TCPConnection::send(void* data) const
	{
		if(data == nullptr)
		{
			return false;
		}
		return true;
	}
}
