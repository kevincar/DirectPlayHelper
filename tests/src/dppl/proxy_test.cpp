#include <experimental/net>

#include "dppl/DirectPlayServer.hpp"
#include "dppl/hardware_test.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(ProxyTest, join) {
  if (!(hardware_test_check() || test_check("TEST_PROXY_JOIN")))
    return SUCCEED();

  std::experimental::net::io_context io_context;

  GUID instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                   0xc8,       0x4c,   0xef,   0x3c, 0xbb};
  GUID app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
              0xa0,       0x24,   0x76,   0xad, 0x4b};

  std::vector<char> response_data(512, '\0');
  std::shared_ptr<dppl::proxy> host_proxy = std::make_shared<dppl::proxy>(
      &io_context, dppl::proxy::type::host, [&](std::vector<char> buffer) {
        dppl::DPMessage request(&buffer);
        EXPECT_EQ(request.header()->command, 0x5);
        host_proxy->stop();
        std::experimental::net::defer(io_context, [&]() { io_context.stop(); });
      });

  dppl::DirectPlayServer dps(&io_context, [&](std::vector<char> buffer) {
    LOG(DEBUG) << "Direct Play Message Received";
    dppl::DPMessage request(&buffer);
    host_proxy->set_return_addr(
        request.get_return_addr<std::experimental::net::ip::tcp::endpoint>());

    // Normally this would get sent to the DPH server, we're simulating the
    // response here.
    std::u16string session_name = u"Sample Session Name:JK1MP:m10.jkl";
    int session_name_byte_length =
        (session_name.size() + 1) * sizeof(std::u16string::value_type);
    dppl::DPMessage response(&response_data);
    response.header()->cbSize = sizeof(DPMSG_HEADER) +
                                sizeof(DPMSG_ENUMSESSIONSREPLY) +
                                session_name_byte_length;
    response.header()->token = 0xfab;
    // retrn addr doesn't matter here since it will need to be replaced
    // anyway
    response.set_signature();
    response.header()->command = DPSYS_ENUMSESSIONSREPLY;
    response.header()->version = 0xe;
    DPMSG_ENUMSESSIONSREPLY* msg = response.message<DPMSG_ENUMSESSIONSREPLY>();
    DPSESSIONDESC2* session_desc = reinterpret_cast<DPSESSIONDESC2*>(msg);
    session_desc->dwSize = sizeof(DPSESSIONDESC2);
    session_desc->dwFlags =
        DPSESSIONDESCFLAGS::useping | DPSESSIONDESCFLAGS::noplayerupdates;
    session_desc->guidInstance = instance;
    session_desc->guidApplication = app;
    session_desc->dwMaxPlayers = 4;
    session_desc->dpSessionID = 0x0195fda9;
    session_desc->dwUser1 = 0x005200a4;
    session_desc->dwUser3 = 0x0008000a;
    session_desc->dwUser4 = 0xb4;
    msg->dwNameOffset = 92;
    std::copy(session_name.begin(), session_name.end() + 1,
              reinterpret_cast<char16_t*>(&msg->szSessionName));
    response_data.resize(response.header()->cbSize);
    host_proxy->deliver(response_data);
  });

  LOG(INFO) << "Please attempt to join a session. It Will fail, but this will "
               "check if we get the next packet.";
  io_context.run();
}

TEST(ProxyTest, host) {
  if (!(hardware_test_check() || test_check("TEST_PROXY_HOST")))
    return SUCCEED();
  // Ensure that the DirectPlayServer Doesn't interfere with hosting.

  std::experimental::net::io_context io_context;
  dppl::DirectPlayServer dps(&io_context, [&](std::vector<char> buffer) {
    LOG(DEBUG) << "Direct Play Server received a message";
  });

  LOG(INFO) << "Testing host";
  io_context.run();
}
