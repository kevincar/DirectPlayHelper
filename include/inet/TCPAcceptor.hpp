
#ifndef INET_TCP_ACCEPTOR
#define INET_TCP_ACCEPTOR

#include <vector>
#include "inet/TCPConnection.hpp"

namespace inet
{
	class TCPAcceptor : public TCPConnection
	{

		public:
			typedef std::function<bool(TCPConnection const& conn)> AcceptHandler;
			typedef std::function<bool (TCPConnection const& conn)> ProcessHandler;

			TCPAcceptor(AcceptHandler const& AcceptHandler, ProcessHandler const& ConnectionHandler);

			std::vector<TCPConnection const*> getConnections(void) const;
			void removeConnection(int connectionSocket);
			std::shared_ptr<TCPConnection> accept(void);

			AcceptHandler const getAcceptHandler() const;
			ProcessHandler const getConnectionHandler() const;

		protected:
			std::vector<std::shared_ptr<TCPConnection>> childConnections;
			mutable std::mutex child_mutex;

			AcceptHandler acceptHandler;
			ProcessHandler connectionHandler;
	};
}

#endif /* INET_TCP_ACCEPTOR */
