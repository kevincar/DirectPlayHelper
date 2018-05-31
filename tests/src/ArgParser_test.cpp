#include "gtest/gtest.h"
#include "ArgParser.hpp"

TEST(ArgParserTest, vectorize)
{
	char const* argumentString[] = {
		"--server",
		"--client",
		"192.168.1.1",
		"--port",
		"80",
		"--name",
		"dingus"
	};
	std::vector<std::string> result = ArgParser::vectorize(7, argumentString);

	ASSERT_EQ(static_cast<int>(result.size()), 7);

	EXPECT_EQ(result[0], "--server");
	EXPECT_EQ(result[1], "--client");
	EXPECT_EQ(result[2], "192.168.1.1");
	EXPECT_EQ(result[3], "--port");
	EXPECT_EQ(result[4], "80");
	EXPECT_EQ(result[5], "--name");
	EXPECT_EQ(result[6], "dingus");
}
