
#ifndef NATHP_CLIENT_HPP
#define NATHP_CLIENT_HPP

#include "inet/MasterConnection.hpp"
#include "nathp/constants.hpp"
#include "nathp/Packet.hpp"
#include "nathp/ClientRecord.hpp"

namespace nathp
{
	class Client
	{
		public:
			Client(std::string serverIPAddress, int port = NATHP_PORT, bool start = true);
			
			void connect(void);
			std::vector<ClientRecord> getClientList(void) const;

			int nConnectionRetries = -1;
		private:
			bool connectionHandler(inet::IPConnection const& connection);
			void processPacket(Packet& packet) const noexcept;
			void clearProcStat(void) const noexcept;

			std::mutex mutable proc_mutex;
			std::condition_variable mutable proc_cv;
			std::map<Packet::Command, bool> mutable proc_stat;
			std::vector<unsigned char> mutable proc_rslt;

			inet::IPConnection::ConnectionHandler ch;
			inet::TCPConnection serverConnection;
			std::string serverAddress;
	};
}

#endif /* NATHP_CLIENT_HPP */
