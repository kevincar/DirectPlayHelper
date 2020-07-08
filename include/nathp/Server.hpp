
#ifndef NATHP_SERVER_HPP
#define NATHP_SERVER_HPP

#include "inet/MasterConnection.hpp"

namespace nathp
{
	class Server
	{
		public:
			typedef std::function<bool(char const*, unsigned int)> ProcessHandler;

			Server(inet::TCPAcceptor::AcceptHandler const& acceptHandler, ProcessHandler const& processHandler, unsigned int port);

			std::vector<unsigned int> getClientList(void) const;
			bool connectoToClient(unsigned int clientId);
		private:
			//void processMessage(inet::TCPConnection const& connection, Message& message);

			std::unique_ptr<inet::MasterConnection> pMasterConnection;

			inet::TCPAcceptor::AcceptHandler internalAcceptHandler;
			inet::TCPAcceptor::ProcessHandler internalProcessHandler;
			inet::TCPAcceptor::AcceptHandler externalAcceptHandler;
			ProcessHandler externalProcessHandler;

			unsigned int main_port;
	};
}

#endif /* NATHP_SERVER_HPP */
