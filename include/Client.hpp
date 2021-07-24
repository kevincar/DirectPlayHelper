#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_
#include <experimental/net>
#include <vector>

#include "Message.hpp"

namespace dph {
class Client {
 public:
  Client(
      std::experimental::net::io_context* io_context,
      std::experimental::net::ip::tcp::resolver::results_type const& endpoints,
      std::function<void(std::vector<char>)> callback =
          [](std::vector<char> x) {});

  void request_clients(void);

 private:
  void forward_message(Message const& message);

  /* Message Sending */
  void request_id(void);
  void enumerate_clients(void);

  /* Message Handlers */
  void request_id_reply_handler(Message const& message);
  void enumerate_clients_reply_handler(Message const& message);

  /* General net initializers */
  void receive(void);

  /* General net handlers */
  void write_handler(std::error_code const& ec, std::size_t bytes_transmitted);
  void receive_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);

  void connection_handler(
      std::error_code const& ec,
      std::experimental::net::ip::tcp::endpoint const& endpoint);

  uint32_t id_ = 0;
  std::vector<char> send_buf_;
  std::vector<char> recv_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::socket connection_;
  std::experimental::net::steady_timer request_timer_;
  std::function<void(std::vector<char>)> request_callback_;
};
}  // namespace dph
#endif  // INCLUDE_CLIENT_HPP_
