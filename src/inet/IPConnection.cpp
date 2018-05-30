#include "sys/socket.h"
#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"
#include "inet/IPConnection.hpp"

namespace inet
{
	IPConnection::IPConnection(int type, int protocol)
	{
		std::lock_guard<std::mutex> lock {this->socket_mutex};
		this->socket = std::make_shared<Socket>(AF_INET, type, protocol);
	}

	std::string const IPConnection::getAddressString(void) const
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		return this->srcAddress->getAddressString();
	}

	void IPConnection::setAddress(std::string const& address)
	{
		// Set the address
		std::lock_guard<std::mutex> srcAddr_lock {this->srcAddr_mutex};
		this->srcAddress = std::make_unique<ServiceAddress>(address);
		
		// Bind
		std::lock_guard<std::mutex> sock_lock {this->socket_mutex};
		this->srcAddress->bind(this->socket);
	}
}
