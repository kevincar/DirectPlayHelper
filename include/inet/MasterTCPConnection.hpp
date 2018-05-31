
#ifndef INET_MASTER_TCP_CONNECTION_HPP
#define INET_MASTER_TCP_CONNECTION_HPP

#include <vector>
#include "inet/TCPConnection.hpp"

namespace inet
{
	class MasterTCPConnection : public TCPConnection
	{
		typedef bool (&newConnectionHandler)(std::shared_ptr<TCPConnection>& newTCPConnection);
		public:
			MasterTCPConnection(void);
			void listenForIncomingConnections(newConnectionHandler ncHandler, bool block = false);
			void acceptConnection(std::unique_ptr<TCPConnection>& newTCPConnection);
		private:
			std::vector<std::thread> connectionHandlerThreads;
			std::vector<std::shared_ptr<TCPConnection>> TCPConnections;
	};
}

#endif /* INET_MASTER_TCP_CONNECTION_HPP */
