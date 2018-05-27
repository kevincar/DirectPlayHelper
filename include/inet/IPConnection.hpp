
#ifndef __IP_CONNECTION_HPP__
#define __IP_CONNECTION_HPP__

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
#endif /* __IP_CONNECTION_HPP__ */
