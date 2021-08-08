#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_
#include <experimental/net>
#include <vector>

#include <g3log/g3log.hpp>
#include "ClientRecord.hpp"
#include "Message.hpp"

namespace dph {
class Server {
 public:
  explicit Server(std::experimental::net::io_context* io_context);

 private:
  // Receive Handlers
  void process_message(dph::Message message);
  void process_request_id(dph::Message message);
  void process_forward_message(dph::Message message);

  // Net Calls
  void accept(void);
  void send(uint32_t const id);
  void receive(uint32_t const id);

  // Net Handlers
  void accept_handler(std::error_code const& ec,
                      std::experimental::net::ip::tcp::socket socket);
  void send_handler(std::error_code const& ec, std::size_t bytes_transmitted);
  void receive_handler(std::error_code const& ec, std::size_t bytes_transmitted,
                       uint32_t const id);

  uint16_t const kPort_ = 47625;
  std::vector<char> recv_buf_;
  std::vector<char> send_buf_;
  std::vector<dph::ClientRecord> client_records_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::acceptor server_socket_;
  std::vector<std::experimental::net::ip::tcp::socket> connection_sockets_;
};
}  // namespace dph
#endif  // INCLUDE_SERVER_HPP_
