#include "dp/header.hpp"
#include "dp/packedplayer.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(packedplayer, constructor) {
  std::vector<BYTE> data = TMP_CREATEPLAYER;
  BYTE* data_start =
      reinterpret_cast<BYTE*>(&(*data.begin())) + sizeof(DPMSG_HEADER) + 20;

  dp::packedplayer player(data_start);
  std::vector<BYTE> player_data(data_start, data_start + player.size());

  ASSERT_EQ(player.flags, dp::packedplayer::Flags::PLAYERSENDINGMACHINE);
  ASSERT_EQ(player.player_id, 0x0194fdac);
  ASSERT_EQ(player.player_ids.size(), 0);
  ASSERT_EQ(player.system_id, 0x0197fdad);
  ASSERT_EQ(player.player_version, 0xe);
  ASSERT_EQ(player.parent_id, 0);
  ASSERT_STREQ(player.short_name.c_str(), "Jordan");
  ASSERT_EQ(player.short_name.size(), 6);
  ASSERT_EQ(player.long_name.size(), 0);
  ASSERT_EQ(player.player_data.size(), 0);
  ASSERT_EQ(player.player_ids.size(), 0);
  ASSERT_EQ(player.size(), 0x5e);
  ASSERT_EQ(player.to_vector(), player_data);
}
