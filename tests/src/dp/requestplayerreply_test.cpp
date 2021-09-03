#include "dp/header.hpp"
#include "dp/requestplayerreply.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(requestplayerreply, constructor) {
  std::vector<BYTE> full_message_data = TMP_REQUESTPLAYERREPLY_1;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + full_message_data.size());

  dp::requestplayerreply message(message_data.data());

  ASSERT_EQ(message.id, 0x0197fdad);
  ASSERT_EQ(message.result, 0);
  ASSERT_EQ(message.to_vector(), message_data);
}
