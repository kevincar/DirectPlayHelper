#include "dp/deleteplayer.hpp"
#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(deleteplayer, contructor) {
  std::vector<BYTE> full_message_data = TMP_DELETEPLAYER;
  std::vector<BYTE> message_data(
      full_message_data.data() + sizeof(DPMSG_HEADER),
      full_message_data.data() + full_message_data.size());

  dp::deleteplayer message(message_data.data());
  ASSERT_EQ(message.player_id, 0x0194fdac);
  ASSERT_EQ(message.to_vector(), message_data);
}
