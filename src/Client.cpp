#include <g3log/g3log.hpp>

#include "Client.hpp"

namespace dph {
Client::Client(
    std::experimental::net::io_context* io_context,
    std::experimental::net::ip::tcp::resolver::results_type const& endpoints)
    : io_context_(io_context),
      connection_(*io_context, std::experimental::net::ip::tcp::v4()) {
  auto handler = std::bind(&Client::connection_handler, this,
                           std::placeholders::_1, std::placeholders::_2);
  std::experimental::net::async_connect(this->connection_, endpoints, handler);
}

void Client::connection_handler(
    std::error_code const& ec,
    std::experimental::net::ip::tcp::endpoint const& endpoint) {
  if (!ec) {
    LOG(DEBUG) << "NICE :)";
  } else {
    LOG(WARNING)
        << "Error attempting to connect with the DirectPlayHelper client: "
        << ec.message();
  }
}
}  // namespace dph
