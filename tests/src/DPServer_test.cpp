#include "gtest/gtest.h"
#include "DPServer.hpp"

TEST(DPServerTest, setServer)
{
	int argc = 2;
	char const* argv[] = {
		"DPServer",
		"-s"
	};
	DPServer dps{argc, argv};
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::SERVER);
	EXPECT_EQ(dps.getHostIPAddress(), std::string(""));
	EXPECT_EQ(dps.getConnPort(), 0);
}

TEST(DPServerTest, setClient)
{
	int argc = 3;
	char const* argv[] = {
		"DPServer",
		"--client",
		"192.168.1.1"
	};

	DPServer dps(argc, argv);
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::CLIENT);
	EXPECT_EQ(dps.getHostIPAddress(), std::string("192.168.1.1"));
	EXPECT_EQ(dps.getConnPort(), 0);
}

TEST(DPServerTest, setClientNoHost)
{
	int argc = 2;
	char const* argv[] = {
		"DPServer",
		"-c"
	};

	DPServer dps(argc, argv);
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::NOT_SET);
	EXPECT_EQ(dps.getHostIPAddress(), std::string(""));
	EXPECT_EQ(dps.getConnPort(), 0);
}

TEST(DPServerTest, setServerAndClient)
{
	int argc = 3;
	char const* argv[] = {
		"DPServer",
		"--server",
		"--client"
	};

	DPServer dps(argc, argv);
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::NOT_SET);
	EXPECT_EQ(dps.getHostIPAddress(), "");
	EXPECT_EQ(dps.getConnPort(), 0);
}

TEST(DPServerTest, setPort)
{
	int argc = 5;
	char const* argv[] = {
		"DPServer",
		"--client",
		"192.168.8.8",
		"--port",
		"2300"
	};

	DPServer dps(argc, argv);
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::CLIENT);
	EXPECT_EQ(dps.getHostIPAddress(), "192.168.8.8");
	EXPECT_EQ(dps.getConnPort(), 2300);
}
