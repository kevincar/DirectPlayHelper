
#ifndef INET_UDP_CONNECTION_HPP
#define INET_UDP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class UDPConnection : public IPConnection
	{
		public:
			UDPConnection(void);
			void connect(std::unique_ptr<ServiceAddress>& addr) override;
			bool send(void* data) const override;
	};
}

#endif /* INET_UDP_CONNECTION_HPP */
