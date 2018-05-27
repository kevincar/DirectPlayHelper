
#ifndef INET_TCP_CONNECTION_HPP
#define INET_TCP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class TCPConnection : public IPConnection
	{
		public:
			TCPConnection(void);
			bool send(void* data) override;
	};
}

#endif /* INET_TCP_CONNECTION_HPP */
