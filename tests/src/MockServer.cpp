#include "MockServer.hpp"
#include "dppl/AppSimulator.hpp"
#include "g3log/g3log.hpp"

MockServer::MockServer(std::experimental::net::io_context* io_context)
    : io_context_(io_context),
      server_socket_(*io_context,
                     std::experimental::net::ip::tcp::endpoint(
                         std::experimental::net::ip::tcp::v4(), 0)),
      connection_socket_(*io_context),
      end_timer(*io_context, std::chrono::seconds(4)) {
  this->accept();
}

std::experimental::net::ip::tcp::endpoint const& MockServer::get_endpoint(
    void) {
  return this->server_socket_.local_endpoint();
}

void MockServer::process_request_id(uint32_t const id) {
  LOG(DEBUG) << "REQUESTID";

  // Set up the message
  this->send_buf_.resize(1024);
  dph::Message dph_message_send(0, id, dph::Command::REQUESTIDREPLY, 0,
                                nullptr);
  std::vector<uint8_t> dph_data = dph_message_send.to_vector();

  // Send it back
  LOG(DEBUG) << "Sending back ID: " << id;

  this->send_buf_ = dph_data;
  this->send();
}

void MockServer::process_forward_message(dph::Message message) {
  // Here we simulate the message being sent to all parties and send
  // back the host response. On the real server, we simply forward the
  // message to the appropariate clinet. If the intended recipient is
  // the host, the message is forwarded to all.
  dppl::message proxy_message(message.payload);

  // We will simply simulate the message being processed on some other
  // client here though
  dp::transmission request = proxy_message.data;
  dp::transmission response = dppl::AppSimulator::process_message(request);

  // If the response is a data message than we can wrap this up
  if (!response.is_dp_message()) {
    std::vector<BYTE> response_data = response.to_vector();
    DWORD* ptr = reinterpret_cast<DWORD*>(response_data.data());
    DWORD command = *(ptr + 2);
    switch (command) {
      case 0x20:
      case 0x22:
        // Do nothing
        break;
      case 0x29:
        // this->end_timer.async_wait([&](std::error_code const&
        // ec){this->io_context_->stop();});
        std::experimental::net::defer([&] { this->io_context_->stop(); });
        return;
        break;
      default:
        LOG(FATAL) << "Unrecognized data command: 0x" << std::hex << command;
    }
  }

  dppl::message response_message(response, {99, 0, 0}, proxy_message.from);
  dph::Message return_message(
      response_message.from.clientID, response_message.to.clientID,
      dph::Command::FORWARDMESSAGE, response_message.to_vector());
  this->send_buf_ = return_message.to_vector();
  this->send();
}

void MockServer::process_message(dph::Message message, uint32_t const id) {
  switch (message.command) {
    case dph::Command::REQUESTID: {
      this->process_request_id(id);
    } break;
    case dph::Command::FORWARDMESSAGE: {
      this->process_forward_message(message);
    } break;
    default:
      std::experimental::net::defer([&]() { this->io_context_->stop(); });
  }
}

void MockServer::accept_handler(
    std::error_code const& ec, std::experimental::net::ip::tcp::socket socket) {
  if (!ec) {
    LOG(DEBUG) << "Accepted";
    this->connection_socket_ = std::move(socket);

    // Create the ID (It's just random here)
    int mock_id = 47625;
    this->client_records_.emplace_back(
        mock_id, this->connection_socket_.remote_endpoint());

    this->receive(mock_id);

    this->accept();
  } else {
    LOG(WARNING) << "accept error: " << ec.message();
  }
}

void MockServer::receive_handler(std::error_code const& ec,
                                 std::size_t bytes_transmitted,
                                 uint32_t const id) {
  if (!ec) {
    LOG(DEBUG) << "Data Received!";
    LOG(DEBUG) << this->recv_buf_.size();
    dph::Message dph_message_recv(this->recv_buf_);
    this->process_message(dph_message_recv, id);
  } else {
    LOG(WARNING) << "receive error: " << ec.message();
  }
  this->receive(id);
}

void MockServer::send_handler(std::error_code const& ec,
                              std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Sent bytes";
  } else {
    LOG(WARNING) << "receive error: " << ec.message();
  }
}

void MockServer::accept(void) {
  auto handler = std::bind(&MockServer::accept_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->server_socket_.async_accept(handler);
}

void MockServer::receive(uint32_t const id) {
  this->recv_buf_.resize(1024);
  auto handler = std::bind(&MockServer::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2, id);
  this->connection_socket_.async_receive(
      std::experimental::net::buffer(this->recv_buf_), handler);
}

void MockServer::send(void) {
  auto handler = std::bind(&MockServer::send_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  this->connection_socket_.async_send(
      std::experimental::net::buffer(this->send_buf_), handler);
}
