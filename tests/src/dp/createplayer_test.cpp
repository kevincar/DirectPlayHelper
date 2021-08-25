#include "dp/createplayer.hpp"
#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(createplayer, contructor) {
  std::vector<BYTE> full_message_data = TMP_CREATEPLAYER;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + full_message_data.size());

  dp::createplayer message(message_data.data());
  ASSERT_EQ(message.to_id, 0);
  ASSERT_EQ(message.id, 0x0194fdac);
  ASSERT_EQ(message.group_id, 0x0);
  ASSERT_EQ(message.player.player_id, 0x0194fdac);
  ASSERT_EQ(message.player.system_id, 0x0197fdad);
  ASSERT_STREQ(message.player.short_name.c_str(), "Jordan");
  ASSERT_EQ(message.to_vector(), message_data);
}
