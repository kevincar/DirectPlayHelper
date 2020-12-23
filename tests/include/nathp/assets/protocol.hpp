
#ifndef NATHP_ASSET_PROTOCOL_TEST
#define NATHP_ASSET_PROTOCOL_TEST

#include <string>
#include <mutex>

namespace nathp
{
	namespace asset
	{
		class lock_pack
		{
			public:
				std::string* status;
				std::mutex* status_mutex;
				std::condition_variable* status_cv;
		};
	}
}

#endif /* NATHP_ASSET_PROTOCOL_TEST */
