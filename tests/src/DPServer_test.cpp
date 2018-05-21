#include "gtest/gtest.h"
#include "DPServer.hpp"

TEST(DPServerTest, setServer)
{
	int argc = 2;
	char* argv[] = {
		(char*)"DPServer",
		(char*)"-s"
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
	char* argv[] = {
		(char*)"DPServer",
		(char*)"--client",
		(char*)"192.168.1.1"
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
	char* argv[] = {
		(char*)"DPServer",
		(char*)"-c"
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
	char* argv[] = {
		(char*)"DPServer",
		(char*)"--server",
		(char*)"--client"
	};

	DPServer dps(argc, argv);
	dps.processArgs();
	EXPECT_EQ(dps.getAppState(), DPServer::NOT_SET);
	EXPECT_EQ(dps.getHostIPAddress(), "");
	EXPECT_EQ(dps.getConnPort(), 0);
}
