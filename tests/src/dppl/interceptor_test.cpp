#include "dp/templates.h"
#include "dppl/AppSimulator.hpp"
#include "dppl/hardware_test.hpp"
#include "dppl/interceptor.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

// Helper function for tests below
TEST(interceptorTest, host_test) {
  DWORD peer_id = 5;
  std::vector<BYTE> buf;
  auto dpsrvr_duration = std::chrono::seconds(5);
  auto transmission_duration = std::chrono::milliseconds(750);
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_duration);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      transmission_duration);
  std::shared_ptr<dppl::interceptor> interceptor;

  // Callback Declarations
  std::function<void(std::error_code const&)> dpsrvr_callback;
  std::function<void(dppl::message const&)> dp_callback;
  std::function<void(dppl::message const&)> data_callback;
  std::function<void(dppl::message const&)> send_to_peer;
  std::function<void(std::error_code const&)> internet_callback;
  std::function<void(dppl::message const&)> handle_dp_response;
  std::function<void(dppl::message const&)> handle_data_response;

  // Callback Definitions
  dpsrvr_callback = [&](std::error_code const& ec) {
    if (!ec) {
      LOG(DEBUG) << "dpsrver timer sending data";
      dp::transmission transmission((std::vector<BYTE>(TMP_ENUMSESSIONS)));
      dppl::message proxy_message(transmission, {5, 0, 0}, {0, 0, 0});
      interceptor->dp_deliver(proxy_message);
      dpsrvr_timer.expires_at(std::chrono::steady_clock::now() +
                              dpsrvr_duration);
      dpsrvr_timer.async_wait(dpsrvr_callback);
    } else {
      LOG(DEBUG) << "dpsrvr timer error: " << ec.message();
    }
  };

  internet_callback = [&](std::error_code const& ec) {
    if (!ec) {
      dppl::message request_message(buf);
      dp::transmission response =
          dppl::AppSimulator::process_message(request_message.data);

      // We're sending it back so we swap the to and from ids
      dppl::message response_message(response, request_message.to,
                                     request_message.from);

      if (response_message.data.is_dp_message()) {
        handle_dp_response(response_message);
      } else {
        handle_data_response(response_message);
      }
    } else {
      LOG(DEBUG) << "internet timer error: " << ec.message();
    }
  };

  handle_dp_response = [&](dppl::message const& response_message) {
    DWORD command = response_message.data.msg->header.command;

    LOG(DEBUG) << "Received request from joining peer (id "
               << response_message.from.clientID
               << "). Sending back to interceptor. Received: " << command;
    switch (command) {
      case DPSYS_ENUMSESSIONSREPLY:
      case DPSYS_REQUESTPLAYERID:
      case DPSYS_ADDFORWARDREQUEST:
      case DPSYS_CREATEPLAYER:
        // Nothing to handle
        break;
      default:
        LOG(FATAL) << "Unhandled message from server " << command;
    }
    interceptor->dp_deliver(response_message);
  };

  handle_data_response = [&](dppl::message const& response_message) {
    DWORD id = response_message.from.clientID;
    std::vector<BYTE> data = response_message.data.to_vector();
    DWORD* ptr = reinterpret_cast<DWORD*>(data.data());
    DWORD command = *(ptr + 2);
    LOG(DEBUG) << "Received data from joining peer (id " << id
               << "). Sending back to host data command: " << command;
    interceptor->data_deliver(response_message);
  };

  send_to_peer = [&](dppl::message const& message) {
    LOG(DEBUG) << "Sending over the internet back to the peer";
    buf = message.to_vector();
    internet_timer.expires_at(std::chrono::steady_clock::now() +
                              transmission_duration);
    internet_timer.async_wait(internet_callback);
  };

  dp_callback = [&](dppl::message const& message) {
    LOG(DEBUG) << "interceptor dp callback from " << message.from.clientID;
    DWORD command = message.data.msg->header.command;
    if (command == DPSYS_ENUMSESSIONSREPLY) {
      dpsrvr_timer.cancel();
    }
    // Simulate the send accross the internet!
    send_to_peer(message);
  };

  data_callback = [&](dppl::message const& message) {
    LOG(DEBUG) << "interceptor data callback";
    std::vector<BYTE> data = message.data.to_vector();
    DWORD* ptr = reinterpret_cast<DWORD*>(data.data());
    DWORD command = *(ptr + 2);
    switch (command) {
      case 0x20:
        // Do nothing
        break;
      case 0x29:
        std::experimental::net::defer([&] { io_context.stop(); });
        return;
        break;
      default:
        LOG(FATAL) << "Unhandled data command: " << command;
    }
    send_to_peer(message);
  };

  // Startup
  interceptor = std::make_shared<dppl::interceptor>(&io_context, dp_callback,
                                                    data_callback);

  if (hardware_test_check() || test_check("TEST_INTER_JOIN")) {
    prompt("Please begin to attempt to join a session...");
    io_context.run();
    prompt("Please shut down the app to continue and press enter");
  } else {
    dpsrvr_timer.async_wait(dpsrvr_callback);
    dppl::AppSimulator app(&io_context, true);
    io_context.run();
  }
}

TEST(interceptorTest, join_test) {
  // auto internet_delay = std::chrono::milliseconds(750);
  // std::vector<char> recv_buf;
  // std::vector<char> send_buf;
  // std::experimental::net::io_context io_context;
  // std::experimental::net::steady_timer internet_timer(io_context,
  // internet_delay);
  // std::experimental::net::steady_timer end_timer(io_context,
  // std::chrono::seconds(4));

  // std::shared_ptr<dppl::interceptor> interceptor;
  // std::function<void(std::error_code const&)> internet_callback =
  // [&](std::error_code const& ec) {
  // if (!ec) {
  // dppl::DPProxyMessage recv_proxy_message(recv_buf);
  // std::vector<char> recv_dp_msg = recv_proxy_message.get_dp_msg_data();
  // send_buf = dppl::AppSimulator::process_message(recv_dp_msg);
  // dppl::DPMessage recv_dp_message(&recv_dp_msg);
  // dppl::DPMessage send_dp_message(&send_buf);
  // dppl::DPProxyMessage send_proxy_message(
  // send_buf, recv_proxy_message.get_from_ids(),
  // recv_proxy_message.get_to_ids());

  // if (send_dp_message.header()->command == DPSYS_ENUMSESSIONSREPLY) {
  // // when proxies receive their first message they will initialize
  // // their client_id_ values. For hosts, the first message is a
  // // ENUMSESSIONS message, for clients, it is the ENUMSESSIONSREPLY.
  // // For this test, the from and to ids are 0, because this test
  // // doesn't use an actual client yet, we need to initialize with e
  // // from.clientID field to an arbitrary value to use
  // send_proxy_message.set_from_ids({8, 0, 0});
  // }
  // LOG(DEBUG) << "Received data from host. Command: "
  // << send_dp_message.header()->command;
  // if (send_proxy_message.is_dp_message()) {
  // interceptor->dp_deliver(send_proxy_message.to_vector());
  // } else {
  // interceptor->data_deliver(send_proxy_message.to_vector());
  // }
  // } else {
  // LOG(DEBUG) << "Internet timer error: " << ec.message();
  // }
  // };
  // std::function<void(std::vector<char>)> send_to_internet =
  // [&](std::vector<char> buffer) {
  // LOG(DEBUG) << "Sending data over internet...";
  // recv_buf = buffer;
  // internet_timer.expires_at(internet_timer.expiry() + internet_delay);
  // internet_timer.async_wait(internet_callback);
  // };
  // std::function<void(std::vector<char>)> dp_callback =
  // [&](std::vector<char> buffer) {
  // dppl::DPProxyMessage proxy_message(buffer);
  // std::vector<char> dp_msg = proxy_message.get_dp_msg_data();
  // dppl::DPMessage dp_message(&dp_msg);
  // if (dp_message.header()->command > DPSYS_CREATEPLAYERVERIFY) {
  // dp_message = dppl::DPMessage(&buffer);
  // proxy_message = dppl::DPProxyMessage(buffer, {0, 0}, {0, 0});
  // }
  // LOG(DEBUG) << "interceptor dp callback. Command: "
  // << dp_message.header()->command;
  // send_to_internet(proxy_message.to_vector());
  // };
  // std::function<void(std::vector<char>)> data_callback =
  // [&](std::vector<char> buffer) {
  // LOG(DEBUG) << "interceptor data callback";
  // dppl::DPProxyMessage proxy_message(buffer);
  // std::vector<char> data = proxy_message.get_dp_msg_data();
  // DWORD* ptr = reinterpret_cast<DWORD*>(&(*data.begin()));
  // DWORD command = *(ptr + 2);
  // if (command == 0x22) {
  // std::experimental::net::defer([&]() { io_context.stop(); });
  // }
  // send_to_internet(proxy_message.to_vector());
  // };
  // interceptor = std::make_shared<dppl::interceptor>(&io_context, dp_callback,
  // data_callback);
  // interceptor->set_client_id(3);
  // if (hardware_test_check() || test_check("TEST_INTER_JOIN")) {
  // prompt("Please begin to attempt to join a session...");
  // io_context.run();
  // prompt("Please shut down the app to continue and press enter");
  // } else {
  // dppl::AppSimulator app(&io_context, false);
  // io_context.run();
  // }
}
