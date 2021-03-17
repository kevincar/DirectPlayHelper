#ifndef INCLUDE_DPPL_PROXY_HPP_
#define INCLUDE_DPPL_PROXY_HPP_
#include <experimental/net>
#include <vector>

#include "dppl/DPMessage.hpp"

namespace dppl {
class proxy : public std::enable_shared_from_this<proxy> {
 public:
  enum type { host, peer };

  proxy(std::experimental::net::io_context* io_context, type proxy_type,
        std::function<void(std::vector<char>)> forward);

  void stop();

  std::experimental::net::ip::tcp::endpoint const get_return_addr();
  void set_return_addr(
      std::experimental::net::ip::tcp::endpoint const& app_endpoint);
  void deliver(std::vector<char> const& data);

 private:
  /* Direct Play Socket Processes */
  void dp_accept();
  void dp_accept_handler(std::error_code const& ec,
                         std::experimental::net::ip::tcp::socket new_socket);
  void dp_receive();
  void dp_receive_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);
  void dp_default_receive_handler();
  void dp_send();
  void dp_assert_connection();
  void dp_send_enumsession_handler();
  void dp_send_enumsessionreply_handler();
  void dp_default_send_handler();
  void dp_receipt_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);

  /* App Data Socket Processes */
  void data_receive();
  void data_receive_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted);
  void data_default_receive_handler();
  void data_send();
  void data_send_handler(std::error_code const& ec,
                         std::size_t bytes_transmitted);

  /* Proxy Attributes */
  int localID = -1;  // the ID on the local application
  int realID = -1;   // the ID that is used from upstream messages
  type proxy_type_;

  static int const kBufSize_ = 512;
  std::vector<char> dp_recv_buf_;
  std::vector<char> dp_send_buf_;
  std::vector<char> data_recv_buf_;
  std::vector<char> data_send_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::endpoint app_dp_endpoint_;
  std::experimental::net::ip::tcp::acceptor dp_acceptor_;
  std::experimental::net::ip::tcp::socket dp_send_socket_;
  std::experimental::net::ip::tcp::socket dp_recv_socket_;
  std::experimental::net::ip::udp::socket dpsrvr_socket_;
  std::experimental::net::ip::udp::socket data_socket_;

  std::function<void(std::vector<char>)> forward_;
};

}  // namespace dppl
#endif  // INCLUDE_DPPL_PROXY_HPP_
