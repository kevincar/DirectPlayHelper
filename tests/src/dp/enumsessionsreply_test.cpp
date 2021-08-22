#include "dp/enumsessionsreply.hpp"
#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(enumsessionsreply, constructor) {
  std::vector<BYTE> message_data = TMP_ENUMSESSIONSREPLY;
  BYTE* data =
      reinterpret_cast<BYTE*>(&(*message_data.begin())) + sizeof(DPMSG_HEADER);

  dp::enumsessionsreply message(data);
  ASSERT_STREQ(message.session_name.c_str(), "Kevin's Game:JK1MP:m10.jkl");
}
