
#ifndef INET_TCP_ACCEPTOR
#define INET_TCP_ACCEPTOR

#include <vector>
#include <inet/MasterConnection.hpp>

namespace inet
{
	class TCPAcceptor : public TCPConnection
	{

		public:
			typedef std::function<std::shared_ptr<TCPConnection> (std::shared_ptr<TCPConnection> const& acceptor)> AcceptHandler;

			TCPAcceptor(std::shared_ptr<AcceptHandler> const& AcceptHandler, std::shared_ptr<MasterConnection::processHandler> const& ConnectionHandler);

			std::shared_ptr<std::vector<std::shared_ptr<TCPConnection const>>> getConnections(void) const;

		protected:
			std::vector<std::shared_ptr<TCPConnection>> childConnections;
			mutable std::mutex child_mutex;

			std::shared_ptr<AcceptHandler> acceptHandler;
			std::shared_ptr<MasterConnection::processHandler> connectionHandler;
	};
}

#endif /* INET_TCP_ACCEPTOR */
