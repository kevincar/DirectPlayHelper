#include <thread>
#include "inet/UDPConnection.hpp"
#include "gtest/gtest.h"

TEST(UDPConnection, constructor)
{
	EXPECT_NO_THROW({
			inet::UDPConnection udpc {};
			});
}

TEST(UDPConnection, sendAndRecv)
{
	std::mutex m;
	std::unique_lock<std::mutex> lk {m, std::defer_lock};
	std::condition_variable cv;
	std::string worker1_address {};
	std::string status {};

	std::thread worker1 {[&]{
		inet::UDPConnection udpc {};
		worker1_address = udpc.getAddressString();

		status = "worker1 ready";
		cv.notify_one();

	}};

	std::thread worker2 {[&]{
		lk.lock();
		cv.wait(lk, [&]{return status == "worker1 ready";});
		lk.unlock();
		cv.notify_one();
	}};

	worker1.join();
	worker2.join();
}
