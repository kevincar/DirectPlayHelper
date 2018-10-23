
#ifndef INET_IP_CONNECTION_HPP
#define INET_IP_CONNECTION_HPP

#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"
#include <memory>

namespace inet
{

	class IPConnection
	{
		public:
			IPConnection(int type, int protocol);
			IPConnection(int captureRawSocket, int type, int protocol, IPConnection const& parentConnection, sockaddr_in& destAddr);

			virtual ~IPConnection() = default;

			std::string const getAddressString(void) const;
			std::string const getIPAddressString(void) const;
			std::string const getPortString(void) const;
			std::string const getPort(void) const;
			void setAddress(std::string const& address);
			void listen(void);
			bool isDataReady(double timeout) const;
			int connect(std::string addressString);
			int send(char const* data, unsigned int const data_len) const;
			int recv(char* buffer, unsigned int buffer_len);

			operator int const() const;

		protected:
			mutable std::mutex socket_mutex;
			mutable std::mutex srcAddr_mutex;
			mutable std::mutex destAddr_mutex;
			Socket socket;
			ServiceAddress srcAddress {};
			ServiceAddress destAddress {};
			
			void updateSrcAddr(void);
	};
}

#endif /* INET_IP_CONNECTION_HPP */
