
#include "sys/socket.h"
#include "inet/IPConnection.hpp"

namespace inet
{
	IPConnection::IPConnection(int type, int protocol)
	{
		this->socket = std::unique_ptr<Socket>(new Socket(AF_INET, type, protocol));
	}
}
