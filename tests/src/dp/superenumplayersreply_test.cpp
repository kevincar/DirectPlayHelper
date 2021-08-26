#include "dp/header.hpp"
#include "dp/superenumplayersreply.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(superenumplayersreply, constructor) {
  std::vector<BYTE> full_message_data = TMP_SUPERENUMPLAYERSREPLY;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + full_message_data.size());

  dp::superenumplayersreply message(message_data.data());
  ASSERT_STREQ(message.session_name.c_str(), "Kevin's Game:JK1MP:m10.jkl");
  ASSERT_STREQ(message.password.c_str(), "");
  ASSERT_EQ(message.players.size(), 3);
  ASSERT_EQ(message.groups.size(), 0);
  ASSERT_EQ(message.shortcuts.size(), 0);
  ASSERT_STREQ(message.players[2].short_name.c_str(), "Kevin");
  ASSERT_EQ(message.size(), message_data.size());
  ASSERT_EQ(message.to_vector(), message_data);
}
