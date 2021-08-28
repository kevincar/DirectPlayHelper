#include "dp/dp.hpp"
#include "dppl/message.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(dppl_message, constructor) {
  std::vector<BYTE> test_data = {0, 1, 2, 3, 4};
  dp::transmission trans(test_data);
  dppl::ENDPOINTIDS toIDs = {1, 15, 14};
  dppl::ENDPOINTIDS fromIDs = {2, 81, 82};
  dppl::message message(trans, fromIDs, toIDs);
  std::vector<BYTE> observed = message.to_vector();
  std::vector<BYTE> expected = {1, 0,  0, 0, 15, 0,  0, 0, 14, 0, 0, 0, 2, 0, 0,
                                0, 81, 0, 0, 0,  82, 0, 0, 0,  0, 1, 2, 3, 4};
  EXPECT_EQ(observed, expected);
  EXPECT_EQ(message.data.to_vector(), test_data);
}
