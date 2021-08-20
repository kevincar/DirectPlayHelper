#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(header, constructor) {
  std::vector<BYTE> data = TMP_HEADER;
  dp::header header(reinterpret_cast<BYTE *>(&(*data.begin())));

  // Ensure loading
  ASSERT_EQ(header.size, 0x00169);
  ASSERT_EQ(header.token, dp::header::Token::REMOTE);

  std::experimental::net::ip::tcp::endpoint e_address(
      std::experimental::net::ip::make_address_v4("0.0.0.0"), 2300);
  ASSERT_EQ(header.sock_addr, e_address);

  ASSERT_STREQ(header.signature.c_str(), "play");

  ASSERT_EQ(header.command, 0x29);
  ASSERT_EQ(header.version, 0xe);

  // Retrieval
  ASSERT_EQ(header.to_vector(), data);
}
