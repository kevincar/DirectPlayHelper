#include "dp/header.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(session, constructor) {
  auto data = std::make_shared<std::vector<BYTE>>(
      std::vector<BYTE>(TMP_ENUMSESSIONSREPLY));
  dp::sessiondesc session(
      data, reinterpret_cast<BYTE *>(&(*data->begin())) + sizeof(DPMSG_HEADER));

  // size
  DWORD e_size = 0x50;
  DWORD o_size = session.get_size();
  ASSERT_EQ(o_size, e_size);

  session.set_size(0x40);
  ASSERT_EQ(session.get_size(), 0x040);

  // flags
  DPSESSIONDESCFLAGS e_flags = DPSESSIONDESCFLAGS::useping | DPSESSIONDESCFLAGS::noplayerupdates;
  DPSESSIONDESCFLAGS o_flags = session.get_flags();
  ASSERT_EQ(o_flags, e_flags);

  e_flags = e_flags | DPSESSIONDESCFLAGS::canjoin;
  session.set_flags(e_flags);
  ASSERT_EQ(session.get_flags(), e_flags);

  // GUID instance
  GUID e_instance = {
    0x87cdc14a,
    0x15f0,
    0x4721,
    0x8f, 0x94, 0x76, 0xc8,
    0x4c, 0xef, 0x3c, 0xbb
  };
  GUID o_instance = session.get_guid_instance();
  ASSERT_EQ(o_instance, e_instance);

  e_instance = {
    0x87cdc14a,
    0x15f0,
    0x4721,
    0x00, 0x00, 0x00, 0x00,
    0x4c, 0xef, 0x3c, 0xbb
  };
  session.set_guid_instance(e_instance);
  ASSERT_EQ(session.get_guid_instance(), e_instance);

  // GUID Application - Skipping since it is the same as Instance

  // Max players
  DWORD e_max_players = 0x4;
  DWORD o_max_players = session.get_max_players();
  ASSERT_EQ(o_max_players, e_max_players);

  session.set_max_players(10);
  ASSERT_EQ(session.get_max_players(), 10);

  // Current players
  DWORD e_current_players = 0x1;
  DWORD o_current_players = session.get_num_current_players();
  ASSERT_EQ(o_current_players, e_current_players);

  session.set_num_current_players(2);
  ASSERT_EQ(session.get_num_current_players(), 2);

  // Session ID
  DWORD e_session_id = 0x0195fda9;
  DWORD o_session_id = session.get_session_id();
  ASSERT_EQ(o_session_id, e_session_id);

  session.set_session_id(0x12345678);
  ASSERT_EQ(session.get_session_id(), 0x12345678);

  // Reserved2
  DWORD e_reserved_2 = 0x0;
  DWORD o_reserved_2 = session.get_reserved_2();
  ASSERT_EQ(o_reserved_2, e_reserved_2);

  session.set_reserved_2(2);
  ASSERT_EQ(session.get_reserved_2(), 2);

  // User1
  DWORD e_user_1 = 0x5200a4;
  DWORD o_user_1 = session.get_user_1();
  ASSERT_EQ(o_user_1, e_user_1);

  session.set_user_1(0x100040);
  ASSERT_EQ(session.get_user_1(), 0x100040);

  // User2
  DWORD e_user_2 = 0x0;
  DWORD o_user_2 = session.get_user_2();
  ASSERT_EQ(o_user_2, e_user_2);

  session.set_user_2(0x5);
  ASSERT_EQ(session.get_user_2(), 0x5);

  // User3
  DWORD e_user_3 = 0x08000a;
  DWORD o_user_3 = session.get_user_3();
  ASSERT_EQ(o_user_3, e_user_3);

  session.set_user_3(0x04000c);
  ASSERT_EQ(session.get_user_3(), 0x04000c);

  // User4
  DWORD e_user_4 = 0xb4;
  DWORD o_user_4 = session.get_user_4();
  ASSERT_EQ(o_user_4, e_user_4);

  session.set_user_4(0xa2);
  ASSERT_EQ(session.get_user_4(), 0xa2);
}
