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
	std::condition_variable cv;
	std::string worker1_address {};
	std::string worker2_address {};
	std::string status {};

	std::thread worker1 {[&]{
		std::string data {};
		std::unique_lock<std::mutex> lk {m, std::defer_lock};
		inet::UDPConnection udpc {};
		worker1_address = udpc.getAddressString();

		status = "worker1 ready";
		cv.notify_one();
		lk.lock();
		cv.wait(lk, [&]{return status == "worker 2 ready";});

		data = "my data";
		int bytes_sent = udpc.send(data.data(), static_cast<unsigned int>(data.size()));
		ASSERT_EQ(bytes_sent, -1);

		int result = udpc.connect(worker2_address);
		EXPECT_EQ(result, 0) << ("failed to connect to: " + worker2_address);
		bytes_sent = udpc.send(data.data(), static_cast<unsigned int>(data.size()));
		EXPECT_GT(bytes_sent, 0) << ("sending error: " + std::to_string(ERRORCODE));

		status = "data sent";
		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return status == "worker 2 received and sent";});

		char buffer[500] = {};
		udpc.recv(buffer, 500);
		EXPECT_STREQ(buffer, "your data received");

		status = "worker 1 received";
		lk.unlock();
		cv.notify_one();
	}};

	std::thread worker2 {[&]{
		std::unique_lock<std::mutex> lk{m};
		cv.wait(lk, [&]{return status == "worker1 ready";});

		inet::UDPConnection udpc {};
		udpc.setAddress("127.0.0.1:0");
		worker2_address = udpc.getAddressString();
		
		status = "worker 2 ready";
		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return status == "data sent";});

		char buffer[500] = {};
		inet::ServiceAddress sa {};
		udpc.recvFrom(buffer, 500, &sa);
		EXPECT_STREQ(buffer, "my data");

		std::string data = "your data received";
		int bytes_sent = udpc.sendTo(data.data(), static_cast<unsigned>(data.size()), sa);
		EXPECT_GT(bytes_sent, 0);

		status = "worker 2 received and sent";
		lk.unlock();
		cv.notify_one();

		lk.lock();
		cv.wait(lk, [&]{return status == "worker 1 received";});
	}};

	worker1.join();
	worker2.join();
}
