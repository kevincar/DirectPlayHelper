#include "dp/addforwardrequest.hpp"
#include "dp/header.hpp"
#include "dp/templates.h"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(addforwardrequest, constructor) {
  std::vector<BYTE> full_message_data = TMP_ADDFORWARDREQUEST;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + full_message_data.size());

  dp::addforwardrequest message(message_data.data());
  ASSERT_EQ(message.size(), message_data.size());
  ASSERT_EQ(message.to_id, 0);
  ASSERT_EQ(message.id, 0x0197fdad);
  ASSERT_EQ(message.group_id, 0);
  ASSERT_EQ(message.player.player_id, 0x0197fdad);
  ASSERT_STREQ(message.password.c_str(), "");
  ASSERT_EQ(message.tick_count, 0x0195fda9);
  ASSERT_EQ(message.to_vector(), message_data);
}
