
#ifndef INET_TCP_ACCEPTOR
#define INET_TCP_ACCEPTOR

#include <vector>
#include "inet/TCPConnection.hpp"

namespace inet
{
	class TCPAcceptor : public std::enable_shared_from_this<TCPAcceptor>, public TCPConnection
	{

		public:
			typedef std::function<std::shared_ptr<TCPConnection const> (TCPAcceptor const& acceptor)> AcceptHandler;
			typedef std::function<bool (TCPConnection const& conn)> ProcessHandler;

			TCPAcceptor(std::shared_ptr<AcceptHandler> const& AcceptHandler, std::shared_ptr<ProcessHandler> const& ConnectionHandler);

			std::vector<TCPConnection const*> const getConnections(void) const;
			std::shared_ptr<TCPConnection> accept(void);

			AcceptHandler const* getAcceptHandler() const;
			ProcessHandler const* getConnectionHandler() const;

		protected:
			std::vector<std::shared_ptr<TCPConnection>> childConnections;
			mutable std::mutex child_mutex;

			std::shared_ptr<AcceptHandler> acceptHandler;
			std::shared_ptr<ProcessHandler> connectionHandler;
	};
}

#endif /* INET_TCP_ACCEPTOR */
