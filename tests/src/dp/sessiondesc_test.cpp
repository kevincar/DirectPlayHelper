#include "dp/header.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(session, constructor) {
  std::vector<BYTE> data = TMP_ENUMSESSIONSREPLY;
  dp::sessiondesc session(
      &data, reinterpret_cast<BYTE *>(&(*data.begin())) + sizeof(DPMSG_HEADER));
}
