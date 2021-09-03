#include "dp/header.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/templates.h"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(session, constructor) {
  std::vector<BYTE> message_data(TMP_ENUMSESSIONSREPLY);
  std::vector<BYTE> session_data(
      message_data.data() + sizeof(DPMSG_HEADER),
      message_data.data() + sizeof(DPMSG_HEADER) + sizeof(DPSESSIONDESC2));
  dp::sessiondesc session(session_data.data());

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
  std::vector<BYTE> copied = session.to_vector();
  ASSERT_EQ(copied.size(), session_data.size());
  ASSERT_EQ(session.to_vector(), session_data);
}
