
#include <inet/TCPAcceptor.hpp>
#include <memory>
#include <iostream>

namespace inet
{
	TCPAcceptor::TCPAcceptor(AcceptHandler const& AcceptHandler, ProcessHandler const& ConnectionHandler) : acceptHandler(AcceptHandler), connectionHandler(ConnectionHandler) {}

	int TCPAcceptor::getLargestSocket(void) const
	{
		int result = *this;
		std::vector<TCPConnection const*> connections = this->getConnections();

		for(TCPConnection const* curConn : connections)
		{
			int curConnSocket = *curConn;
			if(curConnSocket > result)
			{
				result = curConnSocket;
			}
		}

		return result;
	}

	std::vector<TCPConnection const*> TCPAcceptor::getConnections(void) const
	{
		std::vector<TCPConnection const*> result;
		std::lock_guard<std::mutex> lock {this->child_mutex};

		for(std::unique_ptr<TCPConnection> const& curConn : this->childConnections)
		{
			TCPConnection* pCurConn = &(*curConn);
			result.push_back(pCurConn);
		}

		return result;
	}

	void TCPAcceptor::removeConnection(int connectionSocket)
	{
		std::lock_guard<std::mutex> childLock {this->child_mutex};
		for(std::vector<std::unique_ptr<TCPConnection>>::iterator it = this->childConnections.begin(); it != this->childConnections.end(); )
		{
			std::unique_ptr<TCPConnection> const& curConnection = *it;
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

	TCPConnection const& TCPAcceptor::accept(void)
	{
		sockaddr_in peerAddr;
		socklen_t addrSize = 0;
		this->listen();
		int capturedSocket = ::accept(static_cast<int>(*this), reinterpret_cast<sockaddr *>(&peerAddr), &addrSize);

		if(capturedSocket <= -1)
		{
			int err = errno;
			throw std::out_of_range(std::string("TCPAcceptor::accept - Failed to accept connection ") + std::to_string(err));
		}

		std::unique_ptr<TCPConnection> pNewConn = std::make_unique<TCPConnection>(capturedSocket, *this, peerAddr);

		std::lock_guard<std::mutex> acceptLock {this->acceptHandler_mutex};
		bool accepted = this->acceptHandler(*pNewConn);
		if(accepted)
		{
			std::lock_guard<std::mutex> childLock {this->child_mutex};
			this->childConnections.push_back(std::move(pNewConn));
			return *this->childConnections.at(this->childConnections.size()-1);
		}

		return *pNewConn;
	}

	void TCPAcceptor::checkAndProcessConnections(fd_set const& fdSet)
	{
		// Select must have been called previously

		// Check and Process Self
		if(FD_ISSET(*this, &fdSet) == true)
		{
			bool accepted = false;
			TCPConnection const& newConnection = this->accept();

			std::lock_guard<std::mutex> acceptLock{this->acceptHandler_mutex};
			{
				accepted = this->acceptHandler(newConnection);
			}
		}
		
		// Check and Process Children
		std::lock_guard<std::mutex> childLock {this->child_mutex};
		for(std::unique_ptr<TCPConnection> const& conn : this->childConnections)
		{
			if(FD_ISSET(*conn, &fdSet) == true)
			{
				std::lock_guard<std::mutex> procLock {this->connectionHandler_mutex};
				{
					this->connectionHandler(*conn);
				}
			}
		}
		return;
	}

	TCPAcceptor::AcceptHandler const TCPAcceptor::getAcceptHandler(void) const
	{
		return this->acceptHandler;
	}

	TCPAcceptor::ProcessHandler const TCPAcceptor::getConnectionHandler(void) const
	{
		return this->connectionHandler;
	}
}
