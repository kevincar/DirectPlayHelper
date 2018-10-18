
#include <inet/TCPAcceptor.hpp>
#include <memory>
#include <iostream>

namespace inet
{
	TCPAcceptor::TCPAcceptor(AcceptHandler const& AcceptHandler, std::shared_ptr<ProcessHandler> const ConnectionHandler) : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) {}

	std::vector<TCPConnection const*> TCPAcceptor::getConnections(void) const
	{
		std::vector<TCPConnection const*> result;
		std::lock_guard<std::mutex> lock {this->child_mutex};

		for(std::shared_ptr<TCPConnection> curConn : this->childConnections)
		{
			TCPConnection* pCurConn = &(*curConn);
			result.push_back(pCurConn);
		}

		return result;
	}

	void TCPAcceptor::removeConnection(int connectionSocket)
	{
		for(std::vector<std::shared_ptr<TCPConnection>>::iterator it = this->childConnections.begin(); it != this->childConnections.end(); )
		{
			std::shared_ptr<TCPConnection> curConnection = *it;
			int curConnSocket = static_cast<int>(*curConnection);
			if(curConnSocket == connectionSocket)
			{
				it = this->childConnections.erase(it);
			}
			else
			{
				it++;
			}
		}
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

		std::lock_guard<std::mutex> lock {this->child_mutex};
		this->childConnections.emplace_back(std::make_shared<TCPConnection>(capturedSocket, *this, peerAddr));
		return this->childConnections.at(this->childConnections.size()-1);
	}

	TCPAcceptor::AcceptHandler const TCPAcceptor::getAcceptHandler(void) const
	{
		return this->acceptHandler;
	}

	TCPAcceptor::ProcessHandler const* TCPAcceptor::getConnectionHandler(void) const
	{
		return this->connectionHandler.get();
	}
}
