#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_
#include <experimental/net>
#include <vector>

#include "Message.hpp"
#include "dppl/interceptor.hpp"

namespace dph {
class Client {
 public:
  Client(
      std::experimental::net::io_context* io_context,
      std::experimental::net::ip::tcp::resolver::results_type const& endpoints,
      bool use_localhost = false);

  uint32_t get_id(void) const;
  void dp_deliver(std::vector<uint8_t> const& data);

 private:
  void request_id(void);
  void forward_message(Message const& message);

  // Interceptor callbacks
  void dp_callback(dppl::message const& message);
  void data_callback(dppl::message const& message);

  // Net callbacks
  void connection_handler(
      std::error_code const& ec,
      std::experimental::net::ip::tcp::endpoint const& endpoint);
  void receive_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);
  void send_handler(std::error_code const& ec, std::size_t bytes_transmitted);

  // Net Calls
  void connect(std::experimental::net::ip::tcp::resolver::results_type const&);
  void receive(void);
  void send(void);

  uint32_t id_ = 0;
  std::vector<uint8_t> send_buf_;
  std::vector<uint8_t> recv_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::socket connection_;
  dppl::interceptor interceptor_;
};
}  // namespace dph
#endif  // INCLUDE_CLIENT_HPP_
