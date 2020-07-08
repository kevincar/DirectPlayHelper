
#include <cstdlib>
#include "nathp/Server.hpp"


namespace nathp
{
	Server::Server( inet::TCPAcceptor::AcceptHandler const& acceptHandler, ProcessHandler const& processHandler, unsigned int port) : externalAcceptHandler(acceptHandler), externalProcessHandler(processHandler), main_port(port)
	{ 
		// Initialize the internal master connection that will handle the
		// server data
		this->pMasterConnection = std::make_unique<inet::MasterConnection>();
		this->internalAcceptHandler = [&](inet::TCPConnection const& connection) -> bool
		{
			return this->externalAcceptHandler(connection);
		};

		this->internalProcessHandler = [&](inet::TCPConnection const& connection) -> bool
		{
			unsigned int const buffer_size = 1024*4;
			char buffer[buffer_size] {};
			connection.recv(buffer, buffer_size);
			//Message* message = reinterpret_cast<Message*>(buffer);
			//this->processMessage(message);
			return true;
		};
		inet::TCPAcceptor* tcpa = this->pMasterConnection->createTCPAcceptor(this->internalAcceptHandler, this->internalProcessHandler);

		 // Set the address
		 std::string address = std::string("0.0.0.0:") + std::to_string(this->main_port);
		 tcpa->setAddress(address);
		 tcpa->listen();
	}

	std::vector<unsigned int> Server::getClientList(void) const
	{
		std::vector<unsigned int> result {};
		std::vector<inet::TCPConnection const*> connections = this->pMasterConnection->getAcceptors().at(0)->getConnections();
		for(inet::TCPConnection const* curConn : connections)
		{
			unsigned int connId = static_cast<int>(*curConn);
			result.push_back(connId);
		}
		return result;
	}

	bool Server::connectoToClient(unsigned int clientId)
	{
		return false;
	}

	//void Server::processMessage(inet::TCPConnection const& connection, Message& message)
	//{
		//switch(message.command)
		//{
			//case Command::getClientList:
				//// we need an easier and repeatable way to load data into the message
				////std::vector<unsigned int> clientList = this->getClientList();
				////message.data = (unsigned char[])std::malloc(sizeof(clientList.data()));
				////std::copy(clientList.begin(), clientList.end(), message.data);
				////message.result = true;
				////connection.send(reinterpret_cast<char*>(&message), message.data)
				//break;
		//}
	//}
}
