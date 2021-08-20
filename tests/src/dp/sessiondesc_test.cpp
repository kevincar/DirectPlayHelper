#include "dp/header.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(session, constructor) {
  std::vector<BYTE> data(TMP_ENUMSESSIONSREPLY);
  BYTE *byte_data = reinterpret_cast<BYTE *>(&(*data.begin()));
  BYTE *session_ptr = byte_data + sizeof(DPMSG_HEADER);
  std::vector<BYTE> session_data(session_ptr,
                                 session_ptr + sizeof(DPSESSIONDESC2));
  dp::sessiondesc session(session_ptr);

  dp::sessiondesc::Flags e_flags =
      dp::sessiondesc::Flags::useping | dp::sessiondesc::Flags::noplayerupdates;
  ASSERT_EQ(session.flags, e_flags);
  GUID e_instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                     0xc8,       0x4c,   0xef,   0x3c, 0xbb};
  ASSERT_EQ(session.instance, e_instance);
  ASSERT_EQ(session.max_players, 0x4);
  ASSERT_EQ(session.num_players, 0x1);
  ASSERT_EQ(session.session_id, 0x0195fda9);
  ASSERT_EQ(session.reserved2, 0x0);
  ASSERT_EQ(session.user1, 0x5200a4);
  ASSERT_EQ(session.user2, 0x0);
  ASSERT_EQ(session.user3, 0x08000a);
  ASSERT_EQ(session.user4, 0xb4);
  ASSERT_EQ(session.to_vector(), session_data);
}
