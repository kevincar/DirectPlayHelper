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
  void dp_deliver(std::vector<char> const& data);

 private:
  void forward_message(Message const& message);

  /* Message Sending */
  void request_id(void);

  /* Message Handlers */
  void request_id_reply_handler(Message const& message);

  /* General net initializers */
  void receive(void);

  /* General net handlers */
  void write_handler(std::error_code const& ec, std::size_t bytes_transmitted);
  void receive_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);

  void connection_handler(
      std::error_code const& ec,
      std::experimental::net::ip::tcp::endpoint const& endpoint);

  /* Interceptor callbacks */
  void dp_callback(std::vector<char> const& data);
  void data_callback(std::vector<char> const& data);

  uint32_t id_ = 0;
  std::vector<char> send_buf_;
  std::vector<char> recv_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::socket connection_;
  dppl::interceptor interceptor_;
};
}  // namespace dph
#endif  // INCLUDE_CLIENT_HPP_
