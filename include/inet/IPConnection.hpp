
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
			IPConnection(int captureRawSocket, int type, int protocol, sockaddr_in& captureAddr);
			virtual ~IPConnection() = default;
			std::string const getAddressString(void) const;
			std::string const getIPAddressString(void) const;
			std::string const getPortString(void) const;
			std::string const getPort(void) const;
			void setAddress(std::string const& address);
			virtual bool send(void* data) const = 0;
			void listen(void);

		protected:
			mutable std::mutex socket_mutex;
			mutable std::mutex srcAddr_mutex;
			mutable std::mutex destAddr_mutex;
			std::shared_ptr<Socket> socket;
			std::unique_ptr<ServiceAddress> srcAddress = std::make_unique<ServiceAddress>();
			std::unique_ptr<ServiceAddress> destAddress = std::make_unique<ServiceAddress>();
	};
}

#endif /* INET_IP_CONNECTION_HPP */
