
#ifndef NATHP_ASSET_SERVER_TEST
#define NATHP_ASSET_SERVER_TEST

#include "nathp/Server.hpp"
#include "nathp/assets/protocol.hpp"
#include <g3log/g3log.hpp>

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

				// Wait for Clients to finish
				LOG(INFO) << "Waiting for clients to finish";
				std::unique_lock<std::mutex> status_lock {*lp.status_mutex};
				lp.status_cv->wait(status_lock, [&]{
						LOG(INFO) << "&lp.status == " << (long)lp.status;
						LOG(INFO) << "lp.status == " << *lp.status;
						return *lp.status == "Clients Done";
						});
				status_lock.unlock();

				return true;
			}
		}
	}
}

#endif /* NATHP_ASSET_SERVER_TEST */
