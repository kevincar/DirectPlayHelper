
#include "sys/socket.h"
#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"
#include "inet/IPConnection.hpp"

namespace inet
{
	IPConnection::IPConnection(int type, int protocol)
	{
		this->socket = std::unique_ptr<Socket>(new Socket(AF_INET, type, protocol));
	}

	void IPConnection::setAddress(std::string const& address)
	{
		// Set the address
		this->srcAddress = std::unique_ptr<ServiceAddress>(new ServiceAddress {address});
		
		// Bind
		this->srcAddress->bind(*this->socket);
	}
}
