
#ifndef INET_UDP_CONNECTION_HPP
#define INET_UDP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class UDPConnection : public IPConnection
	{
		public:
			UDPConnection(void);
			bool _send(void* data) const override;
	};
}

#endif /* INET_UDP_CONNECTION_HPP */
