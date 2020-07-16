
#ifndef NATHP_CLIENT_HPP
#define NATHP_CLIENT_HPP

#include "inet/MasterConnection.hpp"
#include "nathp/constants.hpp"
#include "nathp/Packet.hpp"

namespace nathp
{
	class Client
	{
		public:
			Client(std::string serverIPAddress, int port = NATHP_PORT, bool start = true);
			
			void connect(void);
			std::vector<unsigned int> getClientList(void) const;

			int nConnectionRetries = -1;
		private:
			bool connectionHandler(inet::IPConnection const& connection);
			void processPacket(Packet const packet) const noexcept;

			inet::IPConnection::ConnectionHandler ch;
			inet::TCPConnection serverConnection;
			std::string serverAddress;
	};
}

#endif /* NATHP_CLIENT_HPP */
