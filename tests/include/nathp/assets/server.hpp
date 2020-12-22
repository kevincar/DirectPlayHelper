
#ifndef NATHP_ASSET_SERVER_TEST
#define NATHP_ASSET_SERVER_TEST

#include "nathp/Server.hpp"
#include "nathp/assets/protocol.hpp"

namespace nathp 
{
	namespace asset
	{
		namespace server
		{
			// Variables
			bool done = false;
			std::mutex done_mutex;
			std::condition_variable done_cv;

			bool acceptHandler(inet::TCPConnection const& connection)
			{
				// Called when the server is accepting a new connection
				return true;
			}

			bool processHandler(char const* buffer, unsigned int size)
			{
				// Called when the server needs to handle incomming information from a
				// client
				return true;
			}

			bool start(int id, lock_pack& lp)
			{
				// Create the nathp server
				nathp::Server server {acceptHandler, processHandler};

				return true;
			}
		}
	}
}

#endif /* NATHP_ASSET_SERVER_TEST */
