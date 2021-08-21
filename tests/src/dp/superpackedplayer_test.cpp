#include "dp/header.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/superpackedplayer.hpp"
#include "dp/templates.h"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(superpackedplayer, contructor) {
  std::vector<BYTE> message_data = TMP_SUPERENUMPLAYERSREPLY;
  BYTE* start = reinterpret_cast<BYTE*>(&(*message_data.begin()));
  BYTE* data = start + 296;
  BYTE* end = data + 65;
  std::vector<BYTE> player_data(data, end);
  dp::superpackedplayer player(data);

  ASSERT_EQ(player.flags, dp::superpackedplayer::Flags::isingroup |
                              dp::superpackedplayer::Flags::islocalplayer);
  ASSERT_EQ(player.id, 0x0194fda8);
  ASSERT_EQ(player.mask.n_service_provider_size_bytes, 1);
  ASSERT_EQ(player.system_id, 0x0194fda9);
  ASSERT_STREQ(player.short_name.c_str(), "Kevin");
  ASSERT_STREQ(player.long_name.c_str(), "");
  ASSERT_EQ(player.player_data_size, 0);
  ASSERT_EQ(player.player_data.size(), 0);
  ASSERT_EQ(player.dp_address,
            std::experimental::net::ip::tcp::endpoint(
                std::experimental::net::ip::tcp::v4(), 2300));
  ASSERT_EQ(player.data_address,
            std::experimental::net::ip::udp::endpoint(
                std::experimental::net::ip::udp::v4(), 2350));
  ASSERT_EQ(player.num_players, 0);
  ASSERT_EQ(player.player_ids.size(), 0);
  ASSERT_EQ(player.parent_id, 0);
  ASSERT_EQ(player.num_shortcut_ids, 0);
  ASSERT_EQ(player.shortcut_ids.size(), 0);
  ASSERT_EQ(player.size(), player_data.size());

  ASSERT_EQ(player.to_vector(), player_data);
}
