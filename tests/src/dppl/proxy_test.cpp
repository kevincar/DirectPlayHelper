#include <experimental/net>

#include "dp/dp.hpp"
#include "dp/templates.h"
#include "dppl/AppSimulator.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "dppl/hardware_test.hpp"
#include "dppl/proxy.hpp"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

TEST(ProxyTest, constructor_host) {
  // Simply ensure that initializing a host or peer proxy doesn't cause any
  // issues. Because these will asynchronously start waiting for data, we'll
  // create a 750ms timer that will shut everything down

  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(750));
  std::shared_ptr<dppl::AppSimulator> app;
  std::shared_ptr<dppl::proxy> proxy;

  // Callbacks Definitions
  std::function<void(dppl::message const &)> proxy_callback;
  std::function<void(std::error_code const &)> timer_callback;

  // Callback implementations
  proxy_callback = [](dppl::message const &proxy_message) {};

  timer_callback = [&](std::error_code const &ec) {
    ASSERT_EQ(ec.value(), 0);
    io_context.stop();
  };

  // Initialize and add jobs
  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::host,
                                        proxy_callback, proxy_callback);
  timer.async_wait(timer_callback);
  EXPECT_NO_THROW({
    app = std::make_shared<dppl::AppSimulator>(&io_context, false);
    io_context.run();
  });
}

TEST(ProxyTest, constructor_peer) {
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(750));
  std::shared_ptr<dppl::proxy> proxy;

  // Callbacks
  std::function<void(dppl::message const &)> callback;
  std::function<void(std::error_code const &)> timer_callback;

  // Callback implementations
  callback = [](dppl::message const &) {};
  timer_callback = [&](std::error_code const &ec) {
    ASSERT_EQ(ec.value(), 0);
    io_context.stop();
  };

  EXPECT_NO_THROW({
    proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::peer,
                                          callback, callback);
    dppl::AppSimulator app(&io_context, true);
    timer.async_wait(timer_callback);
    io_context.run();
  });
}

TEST(ProxyTest, dp_initialization_host) {
  // This test ensures that at host proxy can guide an app all the way through
  // to the DirectPlay protocol initialization
  // Once we receive the DPSYS_CREATEPLAYER message, we've succesffully
  // navigated initialization

  DWORD client_id = 9;
  DWORD system_id = -1;
  DWORD player_id = -1;
  std::vector<BYTE> recv_buf(255, '\0');
  std::vector<BYTE> send_buf(255, '\0');
  std::experimental::net::io_context io_context;
  std::shared_ptr<dppl::proxy> proxy;
  auto const dpsrvr_timer_delay = std::chrono::seconds(5);
  auto const simulated_internet_delay = std::chrono::milliseconds(750);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      simulated_internet_delay);
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_timer_delay);
  std::shared_ptr<dppl::AppSimulator> app;

  // Callback Function Definitions
  std::function<void(std::error_code const &)> dpsrvr_timer_callback;
  std::function<void(std::error_code const &)> internet_timer_callback;
  std::function<void(dppl::message)> send_to_server;
  std::function<void(dppl::message)> dp_callback;
  std::function<void(dppl::message)> data_callback;

  // Callback implementation
  dp_callback = [&](dppl::message message) {
    // We'll use the AppSimulator::process_message to simulate a response
    // over the internet
    LOG(DEBUG) << "Proxy received message from the app";

    // Handl messages with player data that needs to have socket info
    // resolved
    dp::transmission transmitted = message.data;
    dp::header header = transmitted.msg->header;
    LOG(DEBUG) << "Message Command ID: " << header.command;

    // Handle message appropriately
    switch (header.command) {
      case DPSYS_SUPERENUMPLAYERSREPLY: {
        auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
            transmitted.msg->msg);
        for (dp::superpackedplayer &player : msg->players) {
          // Check and handle System Players
          if (player.flags & dp::superpackedplayer::Flags::issystemplayer) {
            LOG(DEBUG) << "Found system player. system_id: " << player.id;

            // Host system player
            if (player.flags & dp::superpackedplayer::Flags::isnameserver) {
              system_id = player.id;
            } else {
              proxy->register_player(&player);
            }
          } else {
            LOG(DEBUG) << "Found application player, player_id: " << player.id;
            if (player.system_id == system_id) {
              player_id = player.id;
            } else {
              proxy->register_player(&player);
            }
          }
        }
      } break;
    }

    LOG(DEBUG) << "Simulating sending this data over the internet to the "
               << "server/other player. Message ID: " << header.command;
    send_to_server(message);
  };

  data_callback = [&](dppl::message message) {
    LOG(DEBUG) << "Proxy_test received a data message from the app";
    std::vector<BYTE> const stream_data = message.data.to_vector();
    DWORD const *ptr = reinterpret_cast<DWORD const *>(stream_data.data());
    DWORD cmd = *(ptr + 2);
    if (cmd == 0x29) {
      ASSERT_EQ(app->is_complete(), true);
      proxy->stop();
      std::experimental::net::post([&]() { io_context.stop(); });
    } else {
      LOG(DEBUG) << "Simulating send this data over the internet to the "
                 << "server/other player. Data ID: " << cmd;
      send_to_server(message);
    }
  };

  dpsrvr_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      LOG(DEBUG) << "dpsrvr timer: simulating ENUMSESSIONS request";
      std::vector<BYTE> data = TMP_ENUMSESSIONS;
      dppl::message message(dp::transmission(data), {1, 0, 0}, {0, 0, 0});
      LOG(DEBUG) << "Sending from: " << message.from.clientID;
      proxy->dp_deliver(message);
      dpsrvr_timer.expires_at(dpsrvr_timer.expiry() + dpsrvr_timer_delay);
      dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  internet_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      dpsrvr_timer.cancel();
      dppl::message proxy_request(recv_buf);
      dp::transmission response =
          dppl::AppSimulator::process_message(proxy_request.data);
      std::string command;
      if (response.is_dp_message()) {
        command =
            std::string("DP") + std::to_string(response.msg->header.command);
      } else {
        std::vector<BYTE> data = response.to_vector();
        DWORD *ptr = reinterpret_cast<DWORD *>(data.data());
        DWORD cmd = *(ptr + 2);
        command = std::string("DATA") + std::to_string(cmd);
      }
      LOG(DEBUG) << "Received response from simulated player with id "
                 << proxy->get_client_id() << ", delivering back message id "
                 << command << " to the app through proxy";
      dppl::message proxy_response(response, proxy->get_ids(),
                                   {client_id, system_id, player_id});
      if (proxy_response.data.is_dp_message()) {
        proxy->dp_deliver(proxy_response);
      } else {
        proxy->data_deliver(proxy_response);
      }
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  send_to_server = [&](dppl::message request) {
    recv_buf = request.to_vector();
    internet_timer.expires_at(std::chrono::steady_clock::now() +
                              simulated_internet_delay);
    internet_timer.async_wait(internet_timer_callback);
  };

  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::peer,
                                        dp_callback, data_callback);

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
  // Variables (Mimic Interceptor)
  DWORD client_id = 9;
  DWORD system_id;
  DWORD player_id;
  DWORD peer_id = 8;
  int playeridflags;
  std::vector<BYTE> recv_buf(512, '\0');
  std::vector<BYTE> send_buf(512, '\0');

  auto simulated_internet_delay = std::chrono::milliseconds(750);
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             simulated_internet_delay);
  std::experimental::net::steady_timer end_timer(io_context,
                                                 std::chrono::seconds(4));
  std::shared_ptr<dppl::AppSimulator> app;
  std::shared_ptr<dppl::proxy> proxy;
  std::shared_ptr<dppl::DirectPlayServer> dps;

  // Callback Definitions
  std::function<dppl::message(dp::transmission)> handle_dp_message;
  std::function<dppl::message(dp::transmission)> handle_data_message;
  std::function<void(std::error_code const &)> internet_callback;
  std::function<void(dppl::message const &)> send_to_server;
  std::function<void(dp::transmission)> dps_callback;
  std::function<void(dppl::message const &)> proxy_dp_callback;
  std::function<void(dppl::message const &)> proxy_data_callback;

  // Callback Implementations
  handle_dp_message = [&](dp::transmission response) {
    DWORD command = response.msg->header.command;
    switch (command) {
      case DPSYS_ENUMSESSIONSREPLY: {
        // The first time a messag comes in from the internet the proxy
        // will not yet have an associated client id, here we set that.
        dppl::message proxy_message(response, {peer_id, 0, 0},
                                    {peer_id, system_id, player_id});
        LOG(DEBUG) << "Sending to " << proxy_message.from.clientID;
        return proxy_message;
      } break;
      case DPSYS_REQUESTPLAYERREPLY: {
        auto msg = std::dynamic_pointer_cast<dp::requestplayerreply>(
            response.msg->msg);
        if (playeridflags & dp::requestplayerid::Flags::issystemplayer) {
          system_id = msg->id;
        } else {
          player_id = msg->id;
        }
        return dppl::message(response, proxy->get_ids(),
                             {peer_id, system_id, player_id});
      } break;
      case DPSYS_ADDFORWARDREQUEST:
        // Nothing to do
        break;
      case DPSYS_SUPERENUMPLAYERSREPLY: {
        // DPMSG_SUPERENUMPLAYERSREPLY *msg =
        auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
            response.msg->msg);
        for (dp::superpackedplayer &player : msg->players) {
          if (player.flags & dp::superpackedplayer::Flags::isnameserver) {
            proxy->register_player(&player);
          } else {
            LOG(DEBUG) << "HERE4";
            if (player.id == system_id || player.system_id == system_id) {
              LOG(DEBUG) << "Found our player";
            } else {
              LOG(DEBUG) << "Found unregistered player";
            }
          }
        }
        return dppl::message(response, proxy->get_ids(),
                             {peer_id, system_id, player_id});
      } break;
      default:
        LOG(FATAL) << "Unhandled DP Message command " << command;
    }
  };

  handle_data_message = [&](dp::transmission response) {
    std::vector<BYTE> data = response.to_vector();
    DWORD *ptr = reinterpret_cast<DWORD *>(data.data());
    DWORD command = *(ptr + 2);
    switch (command) {
      case 0x29: {
        proxy->stop();
        end_timer.async_wait([&](std::error_code const &ec) {
          // ASSERT_EQ(app->is_complete(), true);
          io_context.stop();
        });
      } break;
      case 0x20:
        // Nothing to do
        break;
      default:
        LOG(FATAL) << "Unhandled data message command " << command;
    }
    return dppl::message(response, proxy->get_ids(),
                         {peer_id, system_id, player_id});
  };

  internet_callback = [&](std::error_code const &ec) {
    LOG(DEBUG) << "Received response from internet";
    if (!ec) {
      dppl::message proxy_message(recv_buf);
      dp::transmission request = proxy_message.data;
      dp::transmission response = dppl::AppSimulator::process_message(request);
      LOG(DEBUG)
          << "Received a message from the server. Sending to app via proxy";
      if (response.is_dp_message()) {
        dppl::message proxy_message = handle_dp_message(response);
        LOG(DEBUG) << "Sending to " << proxy_message.from.clientID;
        proxy->dp_deliver(proxy_message);
      } else {
        dppl::message proxy_message = handle_data_message(response);
        LOG(DEBUG) << "Sending data to " << proxy_message.from.clientID;
        proxy->data_deliver(proxy_message);
      }
    } else {
      LOG(DEBUG) << "Timmer Error: " << ec.message();
    }
  };

  send_to_server = [&](dppl::message proxy_message) {
    LOG(DEBUG) << "Sending message over internet";
    recv_buf = proxy_message.to_vector();
    timer.expires_at(std::chrono::steady_clock::now() +
                     simulated_internet_delay);
    timer.async_wait(internet_callback);
  };

  dps_callback = [&](dp::transmission request) {
    LOG(DEBUG) << "Received DPS MESSAGE";
    proxy->set_return_addr(request.msg->header.sock_addr);
    dppl::message proxy_message(request, proxy->get_ids(), {0, 0, 0});
    send_to_server(proxy_message);
  };

  proxy_dp_callback = [&](dppl::message const &message) {
    LOG(DEBUG) << "Received message from proxy";
    DWORD command = message.data.msg->header.command;
    switch (command) {
      case DPSYS_REQUESTPLAYERID: {
        auto msg = std::dynamic_pointer_cast<dp::requestplayerid>(
            message.data.msg->msg);
        playeridflags = static_cast<DWORD>(msg->flags);
      } break;
      case DPSYS_ADDFORWARDREQUEST:
      case DPSYS_CREATEPLAYER:
        // Nothing to handle here
        break;
      default:
        LOG(FATAL) << "Message not handled. Command " << command;
    }
    send_to_server(message);
  };

  proxy_data_callback = [&](dppl::message const &message) {
    LOG(DEBUG) << "Received data message from proxy";
    send_to_server(message);
  };

  dps = std::make_shared<dppl::DirectPlayServer>(&io_context, dps_callback);
  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::host,
                                        proxy_dp_callback, proxy_data_callback);

  if (hardware_test_check() || test_check("TEST_PROXY_JOIN")) {
    prompt(
        "Pleast begin attempting to join a game and press enter when "
        "ready...");
    LOG(DEBUG) << "Listening";
    io_context.run();
    prompt("Please shut down the application now");
  } else {
    app = std::make_shared<dppl::AppSimulator>(&io_context, false);
    io_context.run();
  }
}

TEST(ProxyTest, disconnect) {
  DWORD client_id = 9;
  DWORD system_id = -1;
  DWORD player_id = -1;
  std::vector<BYTE> recv_buf(255, '\0');
  std::vector<BYTE> send_buf(255, '\0');
  std::experimental::net::io_context io_context;
  std::shared_ptr<dppl::proxy> proxy;
  auto const dpsrvr_timer_delay = std::chrono::seconds(5);
  auto const simulated_internet_delay = std::chrono::milliseconds(750);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      simulated_internet_delay);
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_timer_delay);
  std::experimental::net::steady_timer kill_timer(io_context);
  std::unique_ptr<dppl::AppSimulator> app;

  // Callback Function Definitions
  std::function<void(std::error_code const &)> dpsrvr_timer_callback;
  std::function<void(std::error_code const &)> internet_timer_callback;
  std::function<void(dppl::message)> send_to_server;
  std::function<void(dppl::message)> dp_callback;
  std::function<void(dppl::message)> data_callback;
  std::function<void(std::error_code const &)> kill_callback;

  // Callback implementation
  dp_callback = [&](dppl::message message) {
    // We'll use the AppSimulator::process_message to simulate a response
    // over the internet
    LOG(DEBUG) << "Proxy received message from the app";

    // Handl messages with player data that needs to have socket info
    // resolved
    dp::transmission transmitted = message.data;
    dp::header header = transmitted.msg->header;
    LOG(DEBUG) << "Message Command ID: " << header.command;

    // Handle message appropriately
    switch (header.command) {
      case DPSYS_SUPERENUMPLAYERSREPLY: {
        auto msg = std::dynamic_pointer_cast<dp::superenumplayersreply>(
            transmitted.msg->msg);
        for (dp::superpackedplayer &player : msg->players) {
          // Check and handle System Players
          if (player.flags & dp::superpackedplayer::Flags::issystemplayer) {
            LOG(DEBUG) << "Found system player. system_id: " << player.id;

            // Host system player
            if (player.flags & dp::superpackedplayer::Flags::isnameserver) {
              system_id = player.id;
            } else {
              proxy->register_player(&player);
            }
          } else {
            LOG(DEBUG) << "Found application player, player_id: " << player.id;
            if (player.system_id == system_id) {
              player_id = player.id;
            } else {
              proxy->register_player(&player);
            }
          }
        }
      } break;
      case DPSYS_DELETEPLAYER: {
        proxy->stop();
        std::experimental::net::defer([&] { io_context.stop(); });
      } break;
    }

    LOG(DEBUG) << "Simulating sending this data over the internet to the "
               << "server/other player. Message ID: " << header.command;
    send_to_server(message);
  };

  data_callback = [&](dppl::message message) {
    LOG(DEBUG) << "Proxy_test received a data message from the app";
    std::vector<BYTE> const stream_data = message.data.to_vector();
    DWORD const *ptr = reinterpret_cast<DWORD const *>(stream_data.data());
    DWORD cmd = *(ptr + 2);
    LOG(DEBUG) << "Simulating send this data over the internet to the "
               << "server/other player. Data ID: " << cmd;
    send_to_server(message);
  };

  dpsrvr_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      LOG(DEBUG) << "dpsrvr timer: simulating ENUMSESSIONS request";
      std::vector<BYTE> data = TMP_ENUMSESSIONS;
      dppl::message message(dp::transmission(data), {1, 0, 0}, {0, 0, 0});
      LOG(DEBUG) << "Sending from: " << message.from.clientID;
      proxy->dp_deliver(message);
      dpsrvr_timer.expires_at(dpsrvr_timer.expiry() + dpsrvr_timer_delay);
      dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  kill_callback = [&](std::error_code const &ec) {
    if (!ec) {
      LOG(DEBUG) << "Killing App";
      app->shutdown();
    } else {
      LOG(WARNING) << "Kill timer error: " << ec.message();
    }
  };

  internet_timer_callback = [&](std::error_code const &ec) {
    if (!ec) {
      dpsrvr_timer.cancel();
      dppl::message proxy_request(recv_buf);
      dp::transmission response =
          dppl::AppSimulator::process_message(proxy_request.data);
      std::string command;
      if (response.is_dp_message()) {
        command =
            std::string("DP") + std::to_string(response.msg->header.command);
      } else {
        std::vector<BYTE> data = response.to_vector();
        DWORD *ptr = reinterpret_cast<DWORD *>(data.data());
        DWORD cmd = *(ptr + 2);
        command = std::string("DATA") + std::to_string(cmd);
      }
      LOG(DEBUG) << "Received response from simulated player with id "
                 << proxy->get_client_id() << ", delivering back message id "
                 << command << " to the app through proxy";
      dppl::message proxy_response(response, proxy->get_ids(),
                                   {client_id, system_id, player_id});
      if (proxy_response.data.is_dp_message()) {
        proxy->dp_deliver(proxy_response);
      } else {
        proxy->data_deliver(proxy_response);
      }
    } else {
      LOG(WARNING) << "Timer error: " << ec.message();
    }
  };

  send_to_server = [&](dppl::message request) {
    recv_buf = request.to_vector();
    internet_timer.expires_at(std::chrono::steady_clock::now() +
                              simulated_internet_delay);
    internet_timer.async_wait(internet_timer_callback);
  };

  proxy = std::make_shared<dppl::proxy>(&io_context, dppl::proxy::type::peer,
                                        dp_callback, data_callback);

  if (hardware_test_check() || test_check("TEST_PROXY_HOST")) {
    prompt(
        "Please begin a host session on your direct play application and press "
        "enter...");
    dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    io_context.run();
    prompt("Please shutdown your application and press enter...");
  } else {
    kill_timer.expires_at(std::chrono::steady_clock::now() +
                          std::chrono::seconds(7));
    kill_timer.async_wait(kill_callback);
    dpsrvr_timer.async_wait(dpsrvr_timer_callback);
    app = std::make_unique<dppl::AppSimulator>(&io_context, true);
    io_context.run();
  }
}
