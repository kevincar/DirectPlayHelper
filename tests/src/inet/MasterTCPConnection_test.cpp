
#include "inet/MasterTCPConnection.hpp"
#include "gtest/gtest.h"

TEST(MasterTCPConnectionTest, constructor)
{
	ASSERT_NO_THROW({
			inet::MasterTCPConnection mtcpc;
			});
}
