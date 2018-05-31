
#ifndef INET_MASTER_TCP_CONNECTION_HPP
#define INET_MASTER_TCP_CONNECTION_HPP

#include "inet/TCPConnection.hpp"

namespace inet
{
	class MasterTCPConnection : public TCPConnection
	{
		typedef bool (&newConnectionHandler)(std::unique_ptr<TCPConnection>& newTCPConnection);
		public:
			void listenForIncomingConnections(newConnectionHandler ncHandler, bool block = false);
			void acceptConnection(std::unique_ptr<TCPConnection>& newTCPConnection);
		private:
			std::thread connectionHandlerThread;
	};
}

#endif /* INET_MASTER_TCP_CONNECTION_HPP */
