
#ifndef NATHP_CLIENT_HPP
#define NATHP_CLIENT_HPP

#include "inet/MasterConnection.hpp"

namespace nathp
{
	class Client
	{
		public:
			Client(std::string serverAddress);
			
			std::vector<unsigned int> getClientList(void) const;
		private:
			inet::TCPConnection serverConnection;

			inet::IPConnection::ConnectionHandler connectionHandler;
	};
}

#endif /* NATHP_CLIENT_HPP */
