#include "dp/header.hpp"
#include "dp/requestplayerid.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(requestplayerid, constructor) {
  std::vector<BYTE> full_message_data = TMP_REQUESTPLAYERID_1;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + sizeof(DPMSG_HEADER) +
          sizeof(DPMSG_REQUESTPLAYERID));

  dp::requestplayerid message(message_data.data());
  ASSERT_EQ(message.flags, dp::requestplayerid::Flags::unknown |
                               dp::requestplayerid::Flags::issystemplayer);
  ASSERT_EQ(message.to_vector(), message_data);
}
