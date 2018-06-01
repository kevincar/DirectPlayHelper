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

	IPConnection::IPConnection(int capture, int type, int protocol, IPConnection& parentConnection, sockaddr_in& destAddr)
	{
		this->socket = std::make_shared<Socket>(capture, AF_INET, type, protocol);
		this->srcAddress = std::make_unique<ServiceAddress>(parentConnection.srcAddress->getAddressString());
		this->destAddress = std::make_unique<ServiceAddress>(destAddr, this->socket);
	}

	std::string const IPConnection::getAddressString(void) const
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		return this->srcAddress->getAddressString();
	}

	std::string const IPConnection::getIPAddressString(void) const
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		return this->srcAddress->getIPAddressString();
	}

	void IPConnection::setAddress(std::string const& address)
	{
		// Set the address
		std::lock_guard<std::mutex> srcAddr_lock {this->srcAddr_mutex};
		this->srcAddress->setAddressString(address);
		
		// Bind
		std::lock_guard<std::mutex> sock_lock {this->socket_mutex};
		this->srcAddress->bind(this->socket);
	}

	void IPConnection::listen(void)
	{
		std::lock_guard<std::mutex> lock {this->srcAddr_mutex};
		this->srcAddress->listen(this->socket);
	}
}
