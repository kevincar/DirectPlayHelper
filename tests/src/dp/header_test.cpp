#include "dp/header.hpp"
#include "dp/templates.h"
#include "gtest/gtest.h"

TEST(header, constructor) {
  std::vector<BYTE> data = TMP_HEADER;
  dp::header header(&data);

  // cbSize
  DWORD e_cbSize = 0x00169;
  DWORD o_cbSize = header.get_cb_size();
  ASSERT_EQ(o_cbSize, e_cbSize);

  header.set_cb_size(0x170);
  ASSERT_EQ(header.get_cb_size(), 0x170);

  // token
  dp::header::token e_token = dp::header::token::REMOTE;
  dp::header::token o_token = header.get_token();
  ASSERT_EQ(o_token, e_token);

  header.set_token(dp::header::token::SERVER);
  ASSERT_EQ(header.get_token(), dp::header::token::SERVER);

  // sockAddr
  std::experimental::net::ip::tcp::endpoint e_address (std::experimental::net::ip::make_address_v4("0.0.0.0"), 2300);
  std::experimental::net::ip::tcp::endpoint o_address = header.get_sock_addr();
  ASSERT_EQ(o_address, e_address);

  std::experimental::net::ip::tcp::endpoint new_endpoint(std::experimental::net::ip::address_v4::loopback(), 1234);
  header.set_sock_addr(new_endpoint);
  ASSERT_EQ(header.get_sock_addr(), new_endpoint);

  // Signature
  std::string e_signature = "play";
  std::string o_signature = header.get_signature();
  ASSERT_EQ(o_signature, e_signature);

  header.set_signature("DANG");
  ASSERT_STREQ(header.get_signature().c_str(), "DANG");

  // Command
  WORD e_command = 0x29;
  WORD o_command = header.get_command();
  ASSERT_EQ(o_command, e_command);

  header.set_command(0x10);
  ASSERT_EQ(header.get_command(), 0x10);

  // Version
  WORD e_version = 0xe;
  WORD o_version = header.get_version();
  ASSERT_EQ(o_version, e_version);

  header.set_version(0x2);
  ASSERT_EQ(header.get_version(), 0x2);
}
