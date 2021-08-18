#include <experimental/net>

#include "dppl/AppSimulator.hpp"
#include "dppl/DPSuperPackedPlayer.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "dppl/dplay.h"
#include "dppl/hardware_test.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(ProxyTest, constructor_host) {
  // Simply ensure that initializing a host or peer proxy doesn't cause any
  // issues. Because these will asynchronously start waiting for data, we'll
  // create a 750ms timer that will shut everything down

  EXPECT_NO_THROW({
    std::experimental::net::io_context io_context;
    std::experimental::net::steady_timer timer(io_context,
                                               std::chrono::milliseconds(750));

    std::function<void(dppl::DPProxyMessage const &)> callback =
        [](dppl::DPProxyMessage const &proxy_message) {};
    std::shared_ptr<dppl::proxy> proxy = std::make_shared<dppl::proxy>(
        &io_context, dppl::proxy::type::host, callback, callback);

    dppl::AppSimulator app(&io_context, false);

    timer.async_wait([&](std::error_code const &ec) {
      ASSERT_EQ(ec.value(), 0);
      io_context.stop();
    });
    io_context.run();
  });
}

TEST(ProxyTest, constructor_peer) {
  EXPECT_NO_THROW({
    std::experimental::net::io_context io_context;
    std::experimental::net::steady_timer timer(io_context,
                                               std::chrono::milliseconds(750));

    std::function<void(dppl::DPProxyMessage const &)> callback =
        [](dppl::DPProxyMessage const &) {};
    std::shared_ptr<dppl::proxy> proxy = std::make_shared<dppl::proxy>(
        &io_context, dppl::proxy::type::peer, callback, callback);

    dppl::AppSimulator app(&io_context, true);

    timer.async_wait([&](std::error_code const &ec) {
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
  DWORD client_id = 9;
  DWORD system_id = -1;
  DWORD player_id = -1;
  std::shared_ptr<dppl::proxy> proxy;
  auto const dpsrvr_timer_delay = std::chrono::seconds(5);
  auto const simulated_internet_delay = std::chrono::milliseconds(750);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      simulated_internet_delay);
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_timer_delay);
  std::shared_ptr<dppl::AppSimulator> app;
  std::function<void(std::error_code const &)> dpsrvr_timer_callback;
  std::function<void(std::error_code const &)> internet_timer_callback;
  std::function<void(std::vector<char> const &)> send_to_server;
  std::function<void(dppl::DPProxyMessage)> proxy_dp_callback =
      [&](dppl::DPProxyMessage buf) {
        // buf.set_from_ids({client_id, system_id, player_id});
        // We'll use the AppSimulator::process_message to simulate a response
        // over the internet
        LOG(DEBUG) << "Proxy received message from the app";
        // Handl messages with player data that needs to have socket info
        // resolved
        dppl::DPMessage response = buf.get_dp_msg();
        LOG(DEBUG) << "Message Command ID: " << response.header()->command;
        switch (response.header()->command) {
          case DPSYS_SUPERENUMPLAYERSREPLY: {
            DPMSG_SUPERENUMPLAYERSREPLY *msg =
                response.message<DPMSG_SUPERENUMPLAYERSREPLY>();
            DPLAYI_SUPERPACKEDPLAYER *player =
                response.property_data<DPLAYI_SUPERPACKEDPLAYER>(
                    msg->dwPackedOffset);
            LOG(DEBUG) << "n players: " << msg->dwPlayerCount;
            for (int i = 0; i < msg->dwPlayerCount; i++) {
              dppl::DPSuperPackedPlayer superpack(player);
              // System player
              if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::issystemplayer) {
                LOG(DEBUG) << "Found system player. system_id: " << player->ID;
                if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::isnameserver) {
                  system_id = player->ID;
                } else {
                  proxy->register_player(player);
                }
              } else {
                LOG(DEBUG) << "Found Application Player, player_id: "
                           << player->ID;
                if (superpack.getSystemPlayerID() == system_id) {
                  player_id = player->ID;
                } else {
                  proxy->register_player(player);
                }
              }
              std::size_t cur_player_size = superpack.size();
              char *next_player_ptr =
                  reinterpret_cast<char *>(player) + cur_player_size;
              player =
                  reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER *>(next_player_ptr);
            }
          } break;
        }

        LOG(DEBUG) << "Simulating sending this data over the internet to the "
                      "server/other player. Message ID: "
                   << buf.get_dp_msg().header()->command;
        send_to_server(buf.to_vector());
      };

  std::function<void(dppl::DPProxyMessage)> proxy_data_callback =
      [&](dppl::DPProxyMessage message) {
        LOG(DEBUG) << "Proxy_test received a data message from the app";
        std::vector<char> stream_data = message.get_dp_msg_data();
        DWORD *ptr = reinterpret_cast<DWORD *>(&(*stream_data.begin()));
        DWORD cmd = *(ptr + 2);
        if (cmd == 0x29) {
          ASSERT_EQ(app->is_complete(), true);
          proxy->stop();
          std::experimental::net::post([&]() { io_context.stop(); });
        } else {
          send_to_server(message.to_vector());
        }
      };

  dpsrvr_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      LOG(DEBUG) << "dpsrvr timer: simulating ENUMSESSIONS request";
      std::vector<uint8_t> data = {
          0x34, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
          0x61, 0x79, 0x02, 0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79,
          0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
          0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
      };
      dppl::DPProxyMessage dpp_msg(data, {0, 0, 0}, {1, 0, 0});
      LOG(DEBUG) << "Sending from: " << dpp_msg.get_from_ids().clientID;
      proxy->dp_deliver(dpp_msg);
      dpsrvr_timer.expires_at(dpsrvr_timer.expiry() + dpsrvr_timer_delay);
      dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  internet_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      dpsrvr_timer.cancel();
      dppl::DPProxyMessage proxy_request_message(recv_buf);
      send_buf = dppl::AppSimulator::process_message(
          proxy_request_message.get_dp_msg_data());
      dppl::DPMessage response_message(&send_buf);
      LOG(DEBUG) << "Received response from simulated player with id "
                 << proxy->get_client_id() << ", delivering back message id "
                 << response_message.header()->command
                 << " to the app through proxy";
      dppl::DPProxyMessage proxy_response_message(
          send_buf, {client_id, system_id, player_id}, *proxy);
      if (proxy_request_message.is_dp_message()) {
        proxy->dp_deliver(proxy_response_message);
      } else {
        proxy->data_deliver(proxy_response_message);
      }
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  send_to_server = [&](std::vector<char> const &data) {
    recv_buf.assign(data.begin(), data.end());
    internet_timer.expires_at(std::chrono::steady_clock::now() +
                              simulated_internet_delay);
    internet_timer.async_wait(internet_timer_callback);
  };

  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::peer,
                                        proxy_dp_callback, proxy_data_callback);

  if (hardware_test_check() || test_check("TEST_PROXY_HOST")) {
    prompt(
        "Please begin a host session on your direct play application and press "
        "enter...");
    dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    io_context.run();
    prompt("Please shutdown your application and press enter...");
  } else {
    dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    app = std::make_shared<dppl::AppSimulator>(&io_context, true);
    io_context.run();
  }
}

TEST(ProxyTest, dp_initialization_join) {
  auto simulated_internet_delay = std::chrono::milliseconds(750);
  DWORD client_id = 9;
  DWORD system_id;
  DWORD player_id;
  DWORD peer_id = 8;
  int playeridflags;
  std::vector<char> recv_buf(512, '\0');
  std::vector<char> send_buf(512, '\0');
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             simulated_internet_delay);
  std::experimental::net::steady_timer end_timer(io_context,
                                                 std::chrono::seconds(4));
  std::shared_ptr<dppl::AppSimulator> app;
  std::shared_ptr<dppl::proxy> proxy;

  std::function<void(std::error_code const &)> internet_callback =
      [&](std::error_code const &ec) {
        if (!ec) {
          dppl::DPProxyMessage proxy_message(recv_buf);
          dppl::DPMessage request = proxy_message.get_dp_msg();
          send_buf = dppl::AppSimulator::process_message(
              proxy_message.get_dp_msg_data());
          LOG(DEBUG)
              << "Received a message from the server. Sending to app via proxy";
          dppl::DPMessage response(&send_buf);
          switch (response.header()->command) {
            case DPSYS_ENUMSESSIONSREPLY: {
              // NOTHING TO HANDLE HERE
              // The first time a messag comes in from the internet the proxy
              // will not yet have an associated client id, here we set that.
              dppl::DPProxyMessage send_proxy_message(
                  send_buf, {client_id, system_id, player_id}, {peer_id, 0, 0});
              LOG(DEBUG) << "Sending to "
                         << send_proxy_message.get_from_ids().clientID;
              proxy->dp_deliver(send_proxy_message);
            } break;
            case DPSYS_REQUESTPLAYERREPLY: {
              DPMSG_REQUESTPLAYERREPLY *msg =
                  response.message<DPMSG_REQUESTPLAYERREPLY>();
              if (playeridflags & REQUESTPLAYERIDFLAGS::issystemplayer) {
                system_id = msg->dwID;
              } else {
                player_id = msg->dwID;
              }
            } break;
            case DPSYS_SUPERENUMPLAYERSREPLY: {
              DPMSG_SUPERENUMPLAYERSREPLY *msg =
                  response.message<DPMSG_SUPERENUMPLAYERSREPLY>();
              DPLAYI_SUPERPACKEDPLAYER *player =
                  response.property_data<DPLAYI_SUPERPACKEDPLAYER>(
                      msg->dwPackedOffset);
              for (int i = 0; i < msg->dwPlayerCount; i++) {
                dppl::DPSuperPackedPlayer superpack(player);
                if (player->dwFlags & SUPERPACKEDPLAYERFLAGS::isnameserver) {
                  proxy->register_player(player);
                } else {
                  if (player->ID == system_id ||
                      superpack.getSystemPlayerID() == system_id) {
                    LOG(DEBUG) << "Found our player :)";
                  } else {
                    LOG(DEBUG) << "Unregsitered player!";
                  }
                }
                std::size_t player_len = superpack.size();
                char *player_ptr = reinterpret_cast<char *>(player);
                player = reinterpret_cast<DPLAYI_SUPERPACKEDPLAYER *>(
                    player_ptr + player_len);
              }
            } break;
            default:
              LOG(DEBUG) << "Unhandled DP Message command "
                         << response.header()->command;
              if (!dppl::AppSimulator::is_dp_message(send_buf)) {
                DWORD *ptr = reinterpret_cast<DWORD *>(&(*send_buf.begin()));
                DWORD cmd = *(ptr + 2);
                if (cmd == 0x29) {
                  proxy->stop();
                  end_timer.async_wait([&](std::error_code const &ec) {
                    //ASSERT_EQ(app->is_complete(), true);
                    io_context.stop();
                  });
                }
                dppl::DPProxyMessage proxy_message(send_buf, {0, 0, 0}, *proxy);
                return proxy->data_deliver(proxy_message);
              } else {
                return;
              }
          }
          dppl::DPProxyMessage send_proxy_message(
              send_buf, {client_id, system_id, player_id}, *proxy);
          LOG(DEBUG) << "Sending to "
                     << send_proxy_message.get_from_ids().clientID;
          proxy->dp_deliver(send_proxy_message);
        } else {
          LOG(DEBUG) << "Timmer Error: " << ec.message();
        }
      };
  std::function<void(std::vector<char>)> send_to_server =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "Sending message over internet";
        recv_buf = buffer;
        timer.expires_at(std::chrono::steady_clock::now() +
                         simulated_internet_delay);
        timer.async_wait(internet_callback);
      };
  std::function<void(std::vector<char>)> dps_callback =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "Received DPS MESSAGE";
        dppl::DPMessage request(&buffer);
        auto return_addr =
            request
                .get_return_addr<std::experimental::net::ip::tcp::endpoint>();
        return_addr.address(std::experimental::net::ip::address_v4::loopback());
        proxy->set_return_addr(return_addr);
        dppl::DPProxyMessage proxy_message(buffer, {0, 0, 0},
                                           {client_id, system_id, player_id});
        std::vector<char> proxy_message_data = proxy_message.to_vector();
        send_to_server(proxy_message_data);
      };
  std::function<void(dppl::DPProxyMessage)> proxy_dp_callback =
      [&](dppl::DPProxyMessage message) {
        LOG(DEBUG) << "Received message from proxy";
        dppl::DPMessage request = message.get_dp_msg();
        switch (request.header()->command) {
          case DPSYS_REQUESTPLAYERID: {
            DPMSG_REQUESTPLAYERID *msg =
                request.message<DPMSG_REQUESTPLAYERID>();
            playeridflags = msg->dwFlags;
          } break;
          case DPSYS_ADDFORWARDREQUEST:
            // Nothing to handle here
            break;
          case DPSYS_CREATEPLAYER:
            // Nothing to handle here
            break;
          default:
            LOG(DEBUG) << "Message not handled. Command "
                       << request.header()->command;
            return;
        }
        send_to_server(message.to_vector());
      };
  std::function<void(dppl::DPProxyMessage const &)> proxy_data_callback =
      [&](dppl::DPProxyMessage const &message) {
        LOG(DEBUG) << "Received data message from proxy";
        send_to_server(message.to_vector());
      };

  dppl::DirectPlayServer dps(&io_context, dps_callback);
  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::host,
                                        proxy_dp_callback, proxy_data_callback);

  if (hardware_test_check() || test_check("TEST_PROXY_JOIN")) {
    prompt(
        "Pleast begin attempting to join a game and press enter when ready...");
    LOG(DEBUG) << "Listening";
    io_context.run();
    prompt("Please shut down the application now");
  } else {
    app = std::make_shared<dppl::AppSimulator>(&io_context, false);
    io_context.run();
  }
}
