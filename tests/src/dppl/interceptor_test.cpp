#include "dppl/hardware_test.hpp"
#include "dppl/interceptor.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(interceptorTest, join) {
  if (!(hardware_test_check() || test_check("TEST_INTER_JOIN")))
    return SUCCEED();
  std::vector<char> response_data(512, '\0');
  std::shared_ptr<std::function<void(std::vector<char> const&)>>
      send_to_internet;

  GUID instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                   0xc8,       0x4c,   0xef,   0x3c, 0xbb};
  GUID app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
              0xa0,       0x24,   0x76,   0xad, 0x4b};
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(100));
  dppl::interceptor interceptor(
      &io_context, [&](std::vector<char> const& buffer) {
        std::vector<char> buf = buffer;
        dppl::DPMessage packet(&buf);
        LOG(DEBUG) << "ETHERIAL SPACE: RECEIVED DATA: MESSAGE ID: "
                   << packet.header()->command;

        // Simulate sending this off to the internet
        (*send_to_internet)(buffer);
      });

  send_to_internet =
      std::make_shared<std::function<void(std::vector<char> const&)>>(
          [&](std::vector<char> const& buffer) {
            timer.async_wait([&](std::error_code const& ec) {
              if (ec) {
                LOG(DEBUG) << "Timer failed: " << ec.message();
                return;
              }

              std::u16string session_name =
                  u"Sample Session Name:JK1MP:m10.jkl";
              int session_name_byte_length = (session_name.size() + 1) *
                                             sizeof(std::u16string::value_type);
              dppl::DPMessage response(&response_data);
              response.header()->cbSize = sizeof(DPMSG_HEADER) +
                                          sizeof(DPMSG_ENUMSESSIONSREPLY) +
                                          session_name_byte_length;
              response.header()->token = 0xfab;
              // retrn addr doesn't matter here since it will need to be
              // replaced anyway
              response.set_signature();
              response.header()->command = DPSYS_ENUMSESSIONSREPLY;
              response.header()->version = 0xe;
              DPMSG_ENUMSESSIONSREPLY* msg =
                  response.message<DPMSG_ENUMSESSIONSREPLY>();
              DPSESSIONDESC2* session_desc =
                  reinterpret_cast<DPSESSIONDESC2*>(msg);
              session_desc->dwSize = sizeof(DPSESSIONDESC2);
              session_desc->dwFlags = DPSESSIONDESCFLAGS::useping |
                                      DPSESSIONDESCFLAGS::noplayerupdates;
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
              interceptor.deliver(response_data);
              timer.expires_at(timer.expiry() +
                               std::chrono::milliseconds(100));
            });
          });

  LOG(INFO) << "Go Ahead";
  io_context.run();
}
