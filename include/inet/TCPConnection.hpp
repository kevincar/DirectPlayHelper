
#ifndef INET_TCP_CONNECTION_HPP
#define INET_TCP_CONNECTION_HPP

#include "inet/IPConnection.hpp"

namespace inet
{
	class TCPConnection : public IPConnection
	{
		typedef void (&newConnectionHandler)(std::unique_ptr<TCPConnection>& newTCPConnection);
		public:
			TCPConnection(void);
			void listenForIncomingConnections(newConnectionHandler ncHandler);
			bool send(void* data) const override;
	};
}

#endif /* INET_TCP_CONNECTION_HPP */
