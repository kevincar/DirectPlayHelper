
#ifndef INET_UDP_CONNECTION_HPP
#define INET_UDP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class UDPConnection : public IPConnection
	{
		public:
			UDPConnection(void);
			bool send(void* data) override;
	};
}

#endif /* INET_UDP_CONNECTION_HPP */
