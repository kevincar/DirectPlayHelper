#include "Server.hpp"

namespace dph {
Server::Server(std::experimental::net::io_context* io_context, uint16_t port)
    : recv_buf_(1024, '\0'),
      send_buf_(1024, '\0'),
      port_(port),
      io_context_(io_context),
      server_socket_(*io_context,
                     std::experimental::net::ip::tcp::endpoint(
                         std::experimental::net::ip::tcp::v4(), port)) {
  LOG(DEBUG) << "Server constructed";
  this->accept();
}

// Receive Handlers
void Server::process_message(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming message";
  MESSAGE* dph_msg = message.get_message();
  switch (dph_msg->msg_command) {
    case dph::Command::REQUESTID:
      this->process_request_id(message);
      break;
    case dph::Command::FORWARDMESSAGE:
      this->process_forward_message(message);
      break;
  }
}

void Server::process_request_id(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming ID reqest";
  uint32_t const from_id = 0;                    // Server ID
  uint32_t const to_id = message.get_from_id();  // Send back

  // No need to send any data since the ID is sent back in the `to_id` location
  dph::Message response_message(from_id, to_id, dph::Command::REQUESTIDREPLY, 0,
                                nullptr);
  std::vector<char> response_data = response_message.to_vector();
  this->send_buf_.resize(1024);
  this->send_buf_.assign(response_data.begin(), response_data.end());
  this->send(to_id);
}

void Server::process_forward_message(dph::Message message) {
  LOG(DEBUG) << "Server processing incoming foward message request";
  std::vector<char> forward_data = message.to_vector();
  this->send_buf_.resize(1024);
  this->send_buf_.assign(forward_data.begin(), forward_data.end());

  // Broadcast
  if (message.get_to_id() == 0) {
    for (uint32_t id = 1; id <= this->connection_sockets_.size(); id++) {
      if (id == message.get_from_id()) continue;  // Skip self
      this->send(id);
    }
  } else {
    this->send(message.get_to_id());
  }
}

// Net Calls
void Server::accept(void) {
  LOG(DEBUG) << "Server accepting new connections";
  auto handler = std::bind(&Server::accept_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->server_socket_.async_accept(handler);
}

void Server::send(uint32_t const id) {
  LOG(DEBUG) << "Server sending data";
  std::experimental::net::ip::tcp::socket& socket =
      this->connection_sockets_.at(id - 1);
  if (!socket.is_open()) return;
  auto handler = std::bind(&Server::send_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  socket.async_send(std::experimental::net::buffer(this->send_buf_), handler);
}

void Server::receive(uint32_t const id) {
  LOG(DEBUG) << "Server listening for data on connection " << id;
  std::experimental::net::ip::tcp::socket& socket =
      this->connection_sockets_.at(id - 1);
  auto handler = std::bind(&Server::receive_handler, this,
                           std::placeholders::_1, std::placeholders::_2, id);
  socket.async_receive(std::experimental::net::buffer(this->recv_buf_),
                       handler);
}

void Server::stop(uint32_t const id) {
  LOG(DEBUG) << "Server stopping client id " << id;
  std::experimental::net::ip::tcp::socket& socket =
      this->connection_sockets_.at(id - 1);

  std::error_code ec;
  socket.cancel(ec);
  if (!!ec) {
    LOG(WARNING) << "Failed to cancel socket operations for client " << id
                 << "Error: " << ec.message();
    return;
  }

  socket.close(ec);
  if (!!ec) {
    LOG(WARNING) << "Failed to close socket for client " << id
                 << "Error: " << ec.message();
  }
}

// Net Handlers
void Server::accept_handler(std::error_code const& ec,
                            std::experimental::net::ip::tcp::socket socket) {
  if (!ec) {
    LOG(DEBUG) << "Server accepting an incoming connection";
    uint32_t const connection_id = this->connection_sockets_.size() + 1;
    this->client_records_.emplace_back(connection_id, socket.remote_endpoint());
    this->connection_sockets_.push_back(std::move(socket));
    this->receive(connection_id);
    this->accept();
  } else {
    LOG(WARNING) << "Server failed to accept connectiosn: " << ec.message();
  }
}

void Server::send_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted) {
  if (!ec) {
    LOG(DEBUG) << "Server sent " << bytes_transmitted << " byte(s) of data";

  } else {
    LOG(WARNING) << "Server failed to send data: " << ec.message();
  }
}

void Server::receive_handler(std::error_code const& ec,
                             std::size_t bytes_transmitted, uint32_t const id) {
  LOG(DEBUG) << "Server received a message for client " << id;
  if (!ec) {
    dph::Message message(this->recv_buf_);
    uint32_t from_id = message.get_from_id();

    // New Connection
    if (from_id == 0) {
      message.set_from_id(id);
      from_id = id;
    }

    // Assert
    if (from_id != id) {
      LOG(FATAL) << "Server received a message from client "
                 << message.get_from_id() << " which is on a different socket";
    }
    this->process_message(message);
  } else {
    LOG(WARNING) << "Server failed to receive message from the client: "
                 << ec.message() << " (" << ec.value() << ")";
    if (ec.value() == 2) {
      this->stop(id);
      return;
    }
  }
  this->receive(id);
}
}  // namespace dph
