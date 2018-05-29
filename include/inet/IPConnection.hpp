
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
			std::unique_ptr<ServiceAddress> srcAddress;
			std::unique_ptr<ServiceAddress> destAddress;

			IPConnection(int type, int protocol);
			virtual ~IPConnection() = default;
			std::string const getAddressString(void) const;
			void setAddress(std::string const& address);
			virtual bool send(void* data) const = 0;

		private:
			std::mutex socket_mutex;
			std::unique_ptr<Socket> socket;
	};
}

#endif /* INET_IP_CONNECTION_HPP */
