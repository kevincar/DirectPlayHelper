#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_
#include <experimental/net>
#include <vector>

#include "DPHMessage.hpp"

namespace dph {
class Client {
 public:
  Client(
      std::experimental::net::io_context* io_context,
      std::experimental::net::ip::tcp::resolver::results_type const& endpoints);

 private:
  /* Message Sending */
  void request_id(void);
  void enumerate_clients(void) const;
  void forward_message(DPHMessage const& message);

  /* Message Handlers */
  void request_id_reply_handler(DPHMessage const& message);
  void enumerate_clients_reply_handler(DPHMessage const& message);

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
};
}  // namespace dph
#endif  // INCLUDE_CLIENT_HPP_
