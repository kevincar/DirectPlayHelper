#include "dp/playerinfomask.hpp"
#include "gtest/gtest.h"

TEST(playerinfomas, conversion) {
  DWORD mask_value = 0x5;
  BYTE* data = reinterpret_cast<BYTE*>(&mask_value);
  dp::playerinfomask mask(data);

  ASSERT_EQ(mask.short_name_present, true);
  ASSERT_EQ(mask.long_name_present, false);
  ASSERT_EQ(mask.n_service_provider_size_bytes, 1);
  ASSERT_EQ(mask.n_player_data_size_bytes, 0);
  ASSERT_EQ(mask.n_player_count_size_bytes, 0);
  ASSERT_EQ(mask.parent_id_present, false);
  ASSERT_EQ(mask.n_shortcut_count_size_bytes, 0);
}
