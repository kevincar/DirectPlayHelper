#include "dppl/interceptor.hpp"

#include "dppl/hardware_test.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

// Helper function for tests below
std::vector<char> process_message(std::vector<char> const& buffer) {
  LOG(DEBUG) << "Processing message";
  std::vector<char> retval(buffer.begin(), buffer.end());
  GUID app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
              0xa0,       0x24,   0x76,   0xad, 0x4b};
  GUID instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                   0xc8,       0x4c,   0xef,   0x3c, 0xbb};
  dppl::DPMessage packet(&retval);
  std::u16string session_name = u"Sample Session Name:JK1MP:m10.jkl";
  int session_name_byte_length =
      (session_name.size() + 1) * sizeof(std::u16string::value_type);

  switch (packet.header()->command) {
    case DPSYS_ENUMSESSIONSREPLY: {
      std::vector<uint8_t> temp = {
          0xa6, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x01, 0x00, 0x0e, 0x00, 0x50, 0x00, 0x00, 0x00, 0xc0,
          0x00, 0x00, 0x00, 0x4a, 0xc1, 0xcd, 0x87, 0xf0, 0x15, 0x21, 0x47,
          0x8f, 0x94, 0x76, 0xc8, 0x4c, 0xef, 0x3c, 0xbb, 0xc0, 0x13, 0x06,
          0xbf, 0x79, 0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76,
          0xad, 0x4b, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01,
          0x00, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x52, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x0a, 0x00, 0x08, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x5c, 0x00,
          0x00, 0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x6e,
          0x00, 0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00, 0x61, 0x00,
          0x6d, 0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b, 0x00, 0x31,
          0x00, 0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00, 0x31, 0x00,
          0x30, 0x00, 0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x00,
          0x00};
      retval.assign(temp.begin(), temp.end());
    } break;
    case DPSYS_ENUMSESSIONS: {
      LOG(DEBUG) << "Handing ENUMSESSIONS";
      dppl::DPMessage response(&retval);
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
      retval.resize(response.header()->cbSize);
    } break;
    case DPSYS_REQUESTPLAYERID: {
      retval.resize(512, '\0');
      LOG(DEBUG) << "Handling REQUESTPLAYERID";
      dppl::DPMessage response(&retval);
      response.header()->cbSize =
          sizeof(DPMSG_HEADER) + sizeof(DPMSG_REQUESTPLAYERREPLY);
      response.header()->token = 0xfab;
      response.set_signature();
      response.header()->command = DPSYS_REQUESTPLAYERREPLY;
      response.header()->version = 0xe;
      DPMSG_REQUESTPLAYERID* reqmsg = packet.message<DPMSG_REQUESTPLAYERID>();
      DPMSG_REQUESTPLAYERREPLY* msg =
          response.message<DPMSG_REQUESTPLAYERREPLY>();
      if (reqmsg->dwFlags &
          static_cast<int>(REQUESTPLAYERIDFLAGS::issystemplayer)) {
        msg->dwID = 0x197fdad;
      } else {
        msg->dwID = 0x194fdac;
      }
      retval.resize(response.header()->cbSize);
    } break;
    case DPSYS_CREATEPLAYER: {
      retval.resize(512, '\0');
      LOG(DEBUG)
          << "Creting player... End of initialization protocol... NICE :)";
    } break;
    case DPSYS_ADDFORWARDREQUEST: {
      LOG(DEBUG) << "Handling ADDFORWARDREQUEST";
      std::vector<uint8_t> data = {
          0x69, 0x01, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x29, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x24, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x50, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x4a, 0xc1,
          0xcd, 0x87, 0xf0, 0x15, 0x21, 0x47, 0x8f, 0x94, 0x76, 0xc8, 0x4c,
          0xef, 0x3c, 0xbb, 0xc0, 0x13, 0x06, 0xbf, 0x79, 0xde, 0xd0, 0x11,
          0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b, 0x04, 0x00, 0x00,
          0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa4,
          0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00,
          0xb4, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69,
          0x00, 0x6e, 0x00, 0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00,
          0x61, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b,
          0x00, 0x31, 0x00, 0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00,
          0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c,
          0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
          0xad, 0xfd, 0x97, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
          0x00, 0x20, 0x02, 0x00, 0x08, 0xfc, 0xc0, 0xa8, 0x01, 0x47, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e,
          0xc0, 0xa8, 0x01, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xa9, 0xfd,
          0x94, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20,
          0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
          0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01,
          0x05, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x94, 0x01, 0x4b, 0x00, 0x65,
          0x00, 0x76, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x20, 0x02,
          0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      retval.assign(data.begin(), data.end());
    } break;
    default:
      LOG(WARNING) << "Unhandled Command: " << packet.header()->command;
  }
  return retval;
}

TEST(interceptorTest, join) {
  if (!(hardware_test_check() || test_check("TEST_INTER_JOIN")))
    return SUCCEED();
  std::vector<char> request_data(512, '\0');
  std::vector<char> response_data(512, '\0');
  std::shared_ptr<std::function<void(std::vector<char> const&)>>
      send_to_internet;

  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(100));
  dppl::interceptor interceptor(
      &io_context,
      [&](std::vector<char> const& buffer) {
        std::vector<char> buf = buffer;
        dppl::DPMessage packet(&buf);
        LOG(DEBUG) << "ETHERIAL SPACE: RECEIVED DATA: MESSAGE ID: "
                   << packet.header()->command;

        // Simulate sending this off to the internet
        (*send_to_internet)(buffer);
      },
      [&](std::vector<char> const& buffer) {
      });

  send_to_internet =
      std::make_shared<std::function<void(std::vector<char> const&)>>(
          [&](std::vector<char> const& buffer) {
            LOG(DEBUG) << "Sending to the internet :)";
            request_data = buffer;
            timer.async_wait([&](std::error_code const& ec) {
              if (ec) {
                LOG(DEBUG) << "Timer failed: " << ec.message();
                return;
              }
              response_data = process_message(request_data);
              LOG(DEBUG) << "Got a response from the internet";
              interceptor.deliver(response_data);
              timer.expires_at(timer.expiry() + std::chrono::milliseconds(100));
            });
          });

  LOG(INFO) << "Go Ahead";
  io_context.run();
}

TEST(interceptorTest, host) {
  if (!(hardware_test_check() || test_check("TEST_INTER_HOST")))
    return SUCCEED();

  // Need this again :)
  GUID app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
              0xa0,       0x24,   0x76,   0xad, 0x4b};
  int simulated_internet_delay = 100;  // In milliseconds

  // Buffers for sending and receiving data
  std::vector<char> request_data(512, '\0');
  std::vector<char> response_data(512, '\0');

  // A pointer to a function defined later for simulating sending data over the
  // internet
  std::shared_ptr<std::function<void(std::vector<char> const&)>>
      send_to_internet;

  std::experimental::net::io_context io_context;

  // Timer for modulating transmission over our simulated internet
  std::experimental::net::steady_timer timer(
      io_context, std::chrono::milliseconds(simulated_internet_delay));

  // Timer for emitting ENUMSESSION requests
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    std::chrono::seconds(5));

  // Interceptor
  dppl::interceptor interceptor(
      &io_context,
      [&](std::vector<char> const& buffer) {
        std::vector<char> buf = buffer;
        dppl::DPMessage packet(&buf);
        LOG(DEBUG) << "ETHERIAL SPACE: RECEIVED DATA: MESSAGE ID: "
                   << packet.header()->command;

        // Sned off to the interet
        (*send_to_internet)(buffer);
      },
      [&](std::vector<char> const& buffer) {
      });

  // function to send stuff off to the internet
  send_to_internet =
      std::make_shared<std::function<void(std::vector<char> const&)>>(
          [&](std::vector<char> const& buffer) {
            LOG(DEBUG) << "Sending off to the internet :)";
            request_data = buffer;

            // This is for receiving data from the "internet"
            timer.async_wait([&](std::error_code const& ec) {
              if (ec) {
                LOG(DEBUG) << "Timer Failed: " << ec.message();
                return;
              }
              response_data = process_message(request_data);
              LOG(DEBUG) << "Incomming data from the internet";

              // If we received an ENUMSESSIONREPLY let's stop our timer
              dppl::DPMessage response(&response_data);
              if (response.header()->command == DPSYS_ENUMSESSIONSREPLY) {
                dpsrvr_timer.cancel();
              }
              interceptor.deliver(response_data);
              timer.expires_at(timer.expiry() + std::chrono::milliseconds(
                                                    simulated_internet_delay));
            });
          });

  // Call back for our DPSRVR timer
  std::function<void(std::error_code const&)> timer_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          response_data.resize(512, '\0');
          dppl::DPMessage request(&response_data);
          request.header()->cbSize =
              sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS);
          request.header()->token = 0xfab;
          request.set_signature();
          request.header()->command = DPSYS_ENUMSESSIONS;
          request.header()->version = 0xe;
          DPMSG_ENUMSESSIONS* msg = request.message<DPMSG_ENUMSESSIONS>();
          msg->guidApplication = app;
          msg->dwFlags = ENUMSESSIONSFLAGS::allsessions |
                         ENUMSESSIONSFLAGS::passwordprotectedsessions;

          response_data.resize(request.header()->cbSize);

          LOG(DEBUG) << "Timer: Sending sample ENUMSESSIONS request";
          interceptor.deliver(response_data);
          dpsrvr_timer.expires_at(dpsrvr_timer.expiry() +
                                  std::chrono::seconds(5));
          dpsrvr_timer.async_wait(timer_callback);
        } else {
          LOG(WARNING) << "Timer Error: " << ec.message();
        }
      };
  dpsrvr_timer.async_wait(timer_callback);

  LOG(INFO) << "Go Host!";
  io_context.run();
}
