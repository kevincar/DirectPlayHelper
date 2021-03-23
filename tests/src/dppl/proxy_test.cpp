#include "dppl/proxy.hpp"

#include <experimental/net>

#include "dppl/AppSimulator.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "dppl/hardware_test.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(ProxyTest, constructor_host) {
  // Simply ensure that initializing a host or peer proxy doesn't cause any
  // issues. Because these will asynchronously start waiting for data, we'll
  // create a 750ms timer that will shut everything down

  ASSERT_NO_THROW({
    std::experimental::net::io_context io_context;
    std::experimental::net::steady_timer timer(io_context,
                                               std::chrono::milliseconds(750));

    std::function<void(std::vector<char>)> callback =
        [](std::vector<char> buf) {};
    std::shared_ptr<dppl::proxy> proxy = std::make_shared<dppl::proxy>(
        &io_context, dppl::proxy::type::host, callback, callback);

    dppl::AppSimulator app(&io_context, false);

    timer.async_wait([&](std::error_code const& ec) {
      ASSERT_EQ(ec.value(), 0);
      io_context.stop();
    });
    io_context.run();
  });
}

TEST(ProxyTest, constructor_peer) {
  ASSERT_NO_THROW({
    std::experimental::net::io_context io_context;
    std::experimental::net::steady_timer timer(io_context,
                                               std::chrono::milliseconds(750));

    std::function<void(std::vector<char>)> callback =
        [](std::vector<char> buf) {};
    std::shared_ptr<dppl::proxy> proxy = std::make_shared<dppl::proxy>(
        &io_context, dppl::proxy::type::peer, callback, callback);

    dppl::AppSimulator app(&io_context, true);

    timer.async_wait([&](std::error_code const& ec) {
      ASSERT_EQ(ec.value(), 0);
      io_context.stop();
    });
    io_context.run();
  });
}

TEST(ProxyTest, dp_initialization_host) {
  // This test ensures that at host proxy can guide an app all the way through
  // to the DirectPlay protocol initialization
  // Once we receive the DPSYS_CREATEPLAYER message, we've succesffully
  // navigated initialization
  std::experimental::net::io_context io_context;

  std::vector<char> recv_buf(255, '\0');
  std::vector<char> send_buf(255, '\0');
  std::shared_ptr<dppl::proxy> proxy;
  auto const dpsrvr_timer_delay = std::chrono::seconds(5);
  auto const simulated_internet_delay = std::chrono::milliseconds(750);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      simulated_internet_delay);
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_timer_delay);
  std::function<void(std::error_code const&)> dpsrvr_timer_callback;
  std::function<void(std::error_code const&)> internet_timer_callback;
  std::function<void(std::vector<char> const&)> send_to_server;
  std::function<void(std::vector<char>)> proxy_dp_callback = [&](std::vector<char>
                                                                  buf) {
    // We'll use the AppSimulator::process_message to simulate a response
    // over the internet
    LOG(DEBUG) << "Proxy received message from the app";

    LOG(DEBUG) << "Simulating sending this data over the internet to the "
                  "server/other player";
    send_to_server(buf);
  };

  std::function<void(std::vector<char>)> proxy_data_callback = [&](std::vector <char> buf) {
    LOG(DEBUG) << "Proxy_test received a data message from the app";
    DWORD* ptr = reinterpret_cast<DWORD*>(&(*buf.begin()));
    ASSERT_EQ(*ptr, proxy->get_host_player_id());
    std::experimental::net::defer([&](){io_context.stop();});
  };

  dpsrvr_timer_callback = [&](std::error_code const& ec) {
    if (!ec) {
      LOG(DEBUG) << "dpsrvr timer: simulating ENUMSESSIONS request";
      std::vector<uint8_t> data = {
          0x34, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x02, 0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79,
          0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
          0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
      };
      send_buf.assign(data.begin(), data.end());
      proxy->deliver(send_buf);
      dpsrvr_timer.expires_at(dpsrvr_timer.expiry() + dpsrvr_timer_delay);
      dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  internet_timer_callback = [&](std::error_code const& ec) {
    if (!ec) {
      dpsrvr_timer.cancel();
      send_buf = dppl::AppSimulator::process_message(recv_buf);
      LOG(DEBUG) << "Received response from simulated player, delivering back "
                    "to the app through proxy";
      proxy->deliver(send_buf);
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  send_to_server = [&](std::vector<char> const& data) {
    recv_buf.assign(data.begin(), data.end());
    internet_timer.expires_at(std::chrono::steady_clock::now() +
                              simulated_internet_delay);
    internet_timer.async_wait(internet_timer_callback);
  };

  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::peer,
                                        proxy_dp_callback, proxy_data_callback);

  dppl::AppSimulator app(&io_context, true);

  dpsrvr_timer.async_wait(dpsrvr_timer_callback);
  io_context.run();
}

TEST(ProxyTest, join) {
  if (!(hardware_test_check() || test_check("TEST_PROXY_JOIN")))
    return SUCCEED();

  std::experimental::net::io_context io_context;

  GUID instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f, 0x94, 0x76,
                   0xc8,       0x4c,   0xef,   0x3c, 0xbb};

  std::vector<char> response_data(512, '\0');
  std::shared_ptr<dppl::proxy> host_proxy = std::make_shared<dppl::proxy>(
      &io_context, dppl::proxy::type::host, [&](std::vector<char> buffer) {
        dppl::DPMessage request(&buffer);
        EXPECT_EQ(request.header()->command, 0x5);
        host_proxy->stop();
        std::experimental::net::defer(io_context, [&]() { io_context.stop(); });
        }, [&](std::vector <char> buffer){

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
    session_desc->guidApplication = dppl::AppSimulator::app;
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

  bool success = false;
  std::vector<char> send_buf(512, '\0');
  std::vector<char> recv_buf(512, '\0');
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::seconds(5));
  std::shared_ptr<dppl::proxy> peer_proxy = std::make_shared<dppl::proxy>(
      &io_context, dppl::proxy::type::peer, [&](std::vector<char> buffer) {
        LOG(DEBUG) << "Peer proxy received a response rom the app";
        dppl::DPMessage response(&buffer);
        peer_proxy->set_return_addr(
            response
                .get_return_addr<std::experimental::net::ip::tcp::endpoint>());
        LOG(DEBUG) << "Request received. Command ID: "
                   << response.header()->command;

        switch (response.header()->command) {
          case DPSYS_ENUMSESSIONSREPLY: {
            timer.cancel();
            // In real life this would be sent over the internet to the other
            // client, here we'll simulate delivering the next part of the
            // protocol
            send_buf.resize(512, '\0');
            dppl::DPMessage request(&send_buf);
            request.header()->cbSize =
                sizeof(DPMSG_HEADER) + sizeof(DPMSG_REQUESTPLAYERID);
            request.header()->token = 0xfab;
            request.set_signature();
            request.header()->command = DPSYS_REQUESTPLAYERID;
            request.header()->version = 0xe;
            DPMSG_REQUESTPLAYERID* msg =
                request.message<DPMSG_REQUESTPLAYERID>();
            msg->dwFlags = REQUESTPLAYERIDFLAGS::issystemplayer;
            send_buf.resize(request.header()->cbSize);
            peer_proxy->deliver(send_buf);
            break;
          }
          case DPSYS_REQUESTPLAYERREPLY:
            success = true;
            io_context.stop();
            break;
        }
      }, [&](std::vector<char> buffer){

      });

  // Ensure that the DirectPlayServer Doesn't interfere with hosting.
  dppl::DirectPlayServer dps(&io_context, [&](std::vector<char> buffer) {
    LOG(DEBUG) << "Direct Play Server received a message";
  });

  std::function<void(std::error_code const&)> timer_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          dppl::DPMessage request(&send_buf);
          request.header()->cbSize =
              sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS);
          request.header()->token = 0xfab;
          request.set_signature();
          request.header()->command = DPSYS_ENUMSESSIONS;
          request.header()->version = 0xe;
          DPMSG_ENUMSESSIONS* msg = request.message<DPMSG_ENUMSESSIONS>();
          msg->guidApplication = dppl::AppSimulator::app;
          msg->dwFlags = ENUMSESSIONSFLAGS::allsessions |
                         ENUMSESSIONSFLAGS::passwordprotectedsessions;

          send_buf.resize(request.header()->cbSize);

          LOG(DEBUG) << "Timer: Sending sample ENUMSESSIONS request";
          peer_proxy->deliver(send_buf);
          timer.expires_at(timer.expiry() + std::chrono::seconds(5));
          timer.async_wait(timer_callback);
        } else {
          LOG(WARNING) << "Timer Error: " << ec.message();
        }
      };
  timer.async_wait(timer_callback);

  LOG(INFO) << "Please host a game";
  io_context.run();
  EXPECT_EQ(success, true);
}
