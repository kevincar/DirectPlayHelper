#include <iostream>
#include "inet/MasterTCPConnection.hpp"

namespace inet
{
	MasterTCPConnection::MasterTCPConnection(void) : TCPConnection() {}

	void MasterTCPConnection::listenForIncomingConnections(MasterTCPConnection::newConnectionHandler& nch, bool newThread)
	{

	}

	void MasterTCPConnection::listen(MasterTCPConnection::newConnectionHandler& nch)
	{
		// Wait for our next connection
		sockaddr_in addr {};
		unsigned int addrsize = sizeof(sockaddr_in);
		int newSocket = ::accept(*this->socket.get(), (sockaddr*)&addr, &addrsize);
		if(newSocket == -1)
		{
			std::cout << "MasterTCPConnection::listen Failed to accept incoming connection " << errno << std::endl;
			return;
		}

		// Assemble the data into a new TCPConnection object
		std::shared_ptr<TCPConnection> newConnection = std::make_shared<TCPConnection>(newSocket, *this, addr);
	}
}
