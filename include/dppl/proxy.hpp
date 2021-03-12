#ifndef INCLUDE_DPPL_PROXY_HPP_
#define INCLUDE_DPPL_PROXY_HPP_

#include "experimental/net"

namespace dppl {
class proxy : public std::enable_shared_from_this<proxy> {
 public:
  enum type { host, peer };

  proxy(std::experimental::net::io_context* io_context, type proxy_type,
        std::function<void(std::vector<char>)> forward);

  void deliver(std::vector<char> const& data);

  void captureDPSocket(std::experimental::net::ip::tcp::socket socket);
  void captureDataSocket(std::experimental::net::ip::udp::socket socket);

  void operator<<(std::experimental::net::ip::tcp::socket socket);
  void operator<<(std::experimental::net::ip::udp::socket socket);

 private:
  void dp_receive();
  void data_receive();

  void dp_send();
  void data_send();

  void dp_receive_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);
  void data_receive_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted);

  void dp_send_handler(std::error_code const& ec,
                       std::size_t bytes_transmitted);
  void data_send_handler(std::error_code const& ec,
                         std::size_t bytes_transmitted);

  void dp_default_message_handler();
  void data_default_message_handler();

  int localID;  // the ID on the local application
  int realID;   // the ID that is used from upstream messages

  static int const kBufSize_ = 512;
  std::vector<char> dp_buf_;
  std::vector<char> data_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::socket dp_socket_;
  std::experimental::net::ip::udp::socket data_socket_;

  std::function<void(std::vector<char>)> forward_;
};

}  // namespace dppl
#endif  // INCLUDE_DPPL_PROXY_HPP_
