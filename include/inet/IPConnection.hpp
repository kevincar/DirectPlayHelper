
#ifndef INET_IP_CONNECTION_HPP
#define INET_IP_CONNECTION_HPP

#include <memory>
#include "inet/Socket.hpp"

namespace inet
{
	class IPConnection
	{
		public:

			IPConnection(int type, int protocol);

		private:
			std::unique_ptr<Socket> socket;
	};
}

#endif /* INET_IP_CONNECTION_HPP */
