#ifndef TESTS_INCLUDE_MOCKSERVER_HPP_
#define TESTS_INCLUDE_MOCKSERVER_HPP_

#include <vector>

#include "ClientRecord.hpp"
#include "Message.hpp"
#include "experimental/net"

class MockServer {
 public:
  explicit MockServer(std::experimental::net::io_context* io_context);

  std::experimental::net::ip::tcp::endpoint const& get_endpoint(void);

 private:
  // Messaging
  void process_request_id(uint32_t const);
  void process_forward_message(dph::Message message);
  void process_message(dph::Message message, uint32_t const);

  // Net handlers
  void accept_handler(std::error_code const&,
                      std::experimental::net::ip::tcp::socket);
  void receive_handler(std::error_code const& ec, std::size_t bytes_transmitted,
                       uint32_t const id);
  void send_handler(std::error_code const&, std::size_t bytes_transmitted);

  // Net Functions
  void accept(void);
  void receive(uint32_t const);
  void send(void);

  std::vector<uint8_t> recv_buf_;
  std::vector<uint8_t> send_buf_;
  std::vector<dph::ClientRecord> client_records_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::acceptor server_socket_;
  std::experimental::net::ip::tcp::socket connection_socket_;
  std::experimental::net::steady_timer end_timer;
};

#endif  // TESTS_INCLUDE_MOCKSERVER_HPP_
