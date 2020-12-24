
#ifndef NATHP_ASSET_CLIENT_TEST
#define NATHP_ASSET_CLIENT_TEST

#include "nathp/assets/protocol.hpp"
#include "nathp/Client.hpp"
#include "gtest/gtest.h"
#include <g3log/g3log.hpp>

namespace nathp
{
	namespace asset
	{
		namespace client
		{
			bool done = false;
			std::mutex done_mutex;
			std::condition_variable done_cv;
			int n_clients = 0;

			bool start(int id, lock_pack& lp, std::function<void(void)> f)
			{
				n_clients++;

				std::string ip_address = "127.0.0.1";
				nathp::Client client {ip_address, NATHP_PORT, false};
				client.nConnectionRetries = 20;

				LOG(INFO) << "Client connecting...";
				EXPECT_NO_THROW({
						client.connect();
						});

				f();

				n_clients--;
				
				LOG(INFO) << "&lp.status == " << (long)lp.status;
				if (n_clients <= 0)
				{
					std::lock_guard<std::mutex> status_lock {*lp.status_mutex};
					*lp.status = "Clients Done";
				}
				lp.status_cv->notify_all();
				LOG(INFO) << "Client finished...";
				LOG(INFO) << "&lp.status == " << (long)lp.status;
				return true;
			}

			void red()
			{
				LOG(INFO) << "RED CLIENT here :)";
			}

			void gold()
			{
				LOG(INFO) << "GOLD CLIENT HERE :)";
			}
		}
	}
}

#endif /* NATHP_ASSET_CLIENT_TEST */
