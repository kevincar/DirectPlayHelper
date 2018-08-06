
#include <inet/TCPAcceptor.hpp>
#include <memory>
#include <iostream>

namespace inet
{
	TCPAcceptor::TCPAcceptor(std::shared_ptr<AcceptHandler> const& AcceptHandler, std::shared_ptr<MasterConnection::processHandler> const& ConnectionHandler) : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) {}

	std::shared_ptr<std::vector<std::shared_ptr<TCPConnection const>>> TCPAcceptor::getConnections(void) const
	{
		typedef std::shared_ptr<TCPConnection const> pTCPConnection;
		typedef std::vector<pTCPConnection> pTCPConnections;

		std::shared_ptr<pTCPConnections> result = std::make_shared<pTCPConnections>();
		for(pTCPConnection curConnection : this->childConnections)
		{
			result->push_back(curConnection);
		}

		result->push_back(std::static_pointer_cast<TCPConnection const>(this->shared_from_this()));

		//std::cout << "TCPAcceptor socket:" << static_cast<unsigned int>(*this) << std::endl;
		//std::cout << "TCPAcceptor socket:" << static_cast<unsigned int>(*(result->at(result->size()-1).get())) << std::endl;

		return result;
	}

	std::shared_ptr<TCPConnection> TCPAcceptor::accept(void)
	{
		sockaddr_in peerAddr;
		socklen_t addrSize = 0;
		int capturedSocket = ::accept(static_cast<int>(*this), reinterpret_cast<sockaddr *>(&peerAddr), &addrSize);

		if(capturedSocket <= -1)
		{
			throw "TCPAcceptor::accept - Failed to accept connection";
		}

		return std::make_shared<TCPConnection>(capturedSocket, *this, peerAddr);
	}
}
