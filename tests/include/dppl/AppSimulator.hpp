#ifndef TESTS_INCLUDE_DPPL_APPSIMULATOR_HPP_
#define TESTS_INCLUDE_DPPL_APPSIMULATOR_HPP_
#include <memory>
#include <vector>

#include "dppl/dplay.h"
#include "experimental/net"
namespace dppl {
class AppSimulator {
 public:
  AppSimulator(std::experimental::net::io_context* io_context, bool host,
               GUID app = AppSimulator::app,
               GUID instance = AppSimulator::instance);

  static std::vector<char> process_message(std::vector<char> message);

  static GUID constexpr app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
                               0xa0,       0x24,   0x76,   0xad, 0x4b};
  static GUID constexpr instance = {0x87cdc14a, 0x15f0, 0x4721, 0x8f,
                                    0x94,       0x76,   0xc8,   0x4c,
                                    0xef,       0x3c,   0xbb};

 private:
  std::vector<char> handle_message(std::vector<char> message);
  void dp_accept();
  void dp_accept_handler(std::error_code const& ec,
                         std::experimental::net::ip::tcp::socket new_socket);
  void dp_receive();
  void dp_receive_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);
  void dp_connect(std::experimental::net::ip::tcp::endpoint const& endpoint);
  void dp_send();
  void dp_send_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);
  void dpsrvr_receive();
  void dpsrvr_receive_handler(std::error_code const& ec,
                              std::size_t bytes_transmitted);
  void dpsrvr_send();
  void dpsrvr_send_handler(std::error_code const& ec,
                           std::size_t bytes_transmitted);
  void dpsrvr_timer_handler(std::error_code const& ec);

  void data_receive();
  void data_receive_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted);
  void data_connect(std::experimental::net::ip::udp::endpoint const& endpoint);
  void data_send();
  void data_send_handler(std::error_code const& ec,
                         std::size_t bytes_transmitted);

  bool hosting_;
  GUID guidInstance_;
  GUID guidApplication_;
  std::vector<char> dp_recv_buf_;
  std::vector<char> dp_send_buf_;
  std::vector<char> data_recv_buf_;
  std::vector<char> data_send_buf_;
  std::vector<char> dpsrvr_recv_buf_;
  std::vector<char> dpsrvr_send_buf_;
  static int const k_dp_port_ = 2300;
  static int const k_data_port_ = 2350;
  static int const k_dpsrvr_port_ = 47624;
  static auto constexpr k_dpsrvr_time_ = std::chrono::seconds(5);
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::endpoint dp_endpoint_;
  std::experimental::net::ip::udp::endpoint dpsrvr_broadcast_;
  std::experimental::net::ip::udp::endpoint data_endpoint_;
  std::experimental::net::ip::tcp::acceptor dp_acceptor_;
  std::experimental::net::ip::tcp::socket dp_recv_socket_;
  std::experimental::net::ip::tcp::socket dp_send_socket_;
  std::unique_ptr<std::experimental::net::ip::udp::socket> dpsrvr_socket_;
  std::experimental::net::ip::udp::socket data_socket_;
  std::experimental::net::steady_timer dpsrvr_timer_;
};
}  // namespace dppl
#endif  // TESTS_INCLUDE_DPPL_APPSIMULATOR_HPP_
