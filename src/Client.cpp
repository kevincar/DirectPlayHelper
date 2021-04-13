#include "Client.hpp"

namespace dph {
Client::Client(
    std::experimental::net::io_context* io_context,
    std::experimental::net::ip::tcp::endpoint const& connection_endpoint)
    : io_context_(io_context),
      connection_(*io_context, std::experimental::net::ip::tcp::v4()) {}
}  // namespace dph
