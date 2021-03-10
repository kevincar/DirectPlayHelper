#include "dppl/probe/host.hpp"

#include <chrono>

#include "dppl/hardware_test.hpp"

#include "dppl/dplay.h"
#include "experimental/net"
#include "gtest/gtest.h"

TEST(ProbeHostTest, check) {
  if (!hardware_test_check()) return SUCCEED();

  GUID app_guid = {
    0xbf0613c0,
    0xde79,
    0x11d0,
    {
      0x99, 0xc9, 0x00, 0xa0,
      0x24, 0x76, 0xad, 0x4b
    }
  };
  std::experimental::net::io_context io_context;
  dppl::probe::host host_probe(&io_context, app_guid);
  bool attempting_host = host_probe.test(std::chrono::seconds(2));
  ASSERT_EQ(attempting_host, false);

  std::cout << "\n\nPlease attempt to host... then hit enter" << std::endl;
  std::string input;
  std::getline(std::cin, input, '\n');
  attempting_host = host_probe.test(std::chrono::seconds(4));
  ASSERT_EQ(attempting_host, true);
}
