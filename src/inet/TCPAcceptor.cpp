
#include <inet/TCPAcceptor.hpp>

namespace inet
{
	TCPAcceptor::TCPAcceptor(std::shared_ptr<AcceptHandler> const& AcceptHandler, std::shared_ptr<MasterConnection::processHandler> const& ConnectionHandler) : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) {}

	std::shared_ptr<std::vector<std::shared_ptr<TCPConnection const>>> TCPAcceptor::getConnections(void) const
	{
		typedef std::shared_ptr<TCPConnection const> pTCPConnection;
		typedef std::vector<pTCPConnection> pTCPConnections;

		pTCPConnections result {};
		for(pTCPConnection curConnection : this->childConnections)
		{
			result.push_back(curConnection);
		}

		pTCPConnection acceptorConnection {this};
		result.push_back(acceptorConnection);

		return std::shared_ptr<pTCPConnections>{&result};
	}
}
