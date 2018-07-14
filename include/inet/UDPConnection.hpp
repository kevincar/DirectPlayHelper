
#ifndef INET_UDP_CONNECTION_HPP
#define INET_UDP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class UDPConnection : public IPConnection
	{
		public:
			UDPConnection(void);
			int sendTo(void const* data, unsigned int const data_len, ServiceAddress const& addr) const;
			int recvFrom(void* buffer, unsigned int const buffer_len, ServiceAddress& addr) const;
			bool _send(void* data) const override;
	};
}

#endif /* INET_UDP_CONNECTION_HPP */
