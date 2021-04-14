#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_
#include <experimental/net>
namespace dph {
class Client {
 public:
  explicit Client(
      std::experimental::net::io_context* io_context,
      std::experimental::net::ip::tcp::resolver::results_type const& endpoints);

 private:
  void connection_handler(
      std::error_code const& ec,
      std::experimental::net::ip::tcp::endpoint const& endpoint);

  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::socket connection_;
};
}  // namespace dph
#endif  // INCLUDE_CLIENT_HPP_
