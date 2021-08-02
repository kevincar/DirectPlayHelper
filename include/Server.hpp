#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_
#include <experimental/net>
#include <vector>

namespace dph {
class Server {
  explicit Server(std::experimental::net::io_context* io_context)
      : io_context_(io_context) {}

 private:
  // Receive Handlers
  void process_message(dph::Message message);
  void process_request_id(dph::Message message);
  void process_forward_message(dph::Message message);

  // Net Calls
  void accpet(void);
  void send(void);
  void receive(void);

  // Net Handlers
  void do_accept(std::error_code const& ec,
                 std::experimental::net::ip::tcp::socket socket);
  void do_send(std::error_code const& ec, std::size_t bytes_transmitted);
  void do_receive(std::error_code const& ec, std::size_t bytes_transmitted);

  std::vector<char> recv_buf_;
  std::vector<char> send_buf_;
  std::vector<dph::ClientRecord> client_records_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::acceptor server_socket_;
  std::vector<std::experimental::net::ip::tcp::socket> connection_sockets_;
}
}  // namespace dph
#endif  // INCLUDE_SERVER_HPP_
