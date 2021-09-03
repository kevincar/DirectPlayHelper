#include "dp/enumsessions.hpp"
#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(enumsessions, contructor) {
  std::vector<BYTE> full_message_data = TMP_ENUMSESSIONS;
  std::vector<BYTE> message_data(
      full_message_data.begin() + sizeof(DPMSG_HEADER),
      full_message_data.end());

  dp::enumsessions message(message_data.data());
  ASSERT_EQ(message.to_vector(), message_data);
}
