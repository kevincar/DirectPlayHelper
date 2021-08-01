#include "dppl/DPProxyMessage.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(DPProxyMessageTest, ProxyToIDs) {
  std::vector<char> test_data = {0, 1, 2, 3, 4};
  dppl::DPProxyEndpointIDs toIDs = {1, 15, 14};
  dppl::DPProxyEndpointIDs fromIDs = {2, 81, 82};
  dppl::DPProxyMessage dpimsg(test_data, toIDs, fromIDs);
  std::vector<char> observed = dpimsg.to_vector();
  std::vector<char> expected = {1, 0,  0, 0, 15, 0,  0, 0, 14, 0, 0, 0, 2, 0, 0,
                                0, 81, 0, 0, 0,  82, 0, 0, 0,  0, 1, 2, 3, 4};
  EXPECT_EQ((observed == expected), true);

  std::vector<char> re_data = dpimsg.get_dp_msg_data();
  EXPECT_EQ((re_data == test_data), true);
}
