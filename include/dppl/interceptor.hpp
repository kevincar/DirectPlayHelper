#ifndef INCLUDE_DPPL_INTERCEPTOR_HPP_
#define INCLUDE_DPPL_INTERCEPTOR_HPP_
#include <memory>
#include <vector>

#include "dppl/DirectPlayServer.hpp"
#include "dppl/dplay.h"
#include "dppl/proxy.hpp"
#include "experimental/net"
namespace dppl {
class interceptor {
 public:
  interceptor(std::experimental::net::io_context* io_context,
              std::function<void(std::vector<char> const&)> forward);

  void deliver(std::vector<char> const& buffer);

 private:
  /* Proxy Helper Funcs */
  inline bool has_proxies();
  std::shared_ptr<proxy> find_peer_proxy(int const& id);
  bool has_free_peer_proxy();
  std::shared_ptr<proxy> get_free_peer_proxy();
  void direct_play_server_callback(std::vector<char> const& buffer);
  void proxy_dp_callback(std::vector<char> const& buffer);
  void proxy_data_callback(std::vector<char> const& buffer);

  /* handlers for messages from above */
  void enumsessions_from_server_handler();
  void superenumplayersreply_from_server_handler();
  std::size_t register_player(DPLAYI_SUPERPACKEDPLAYER* player);

  std::vector<char> send_buf_;
  std::vector<char> recv_buf_;
  std::function<void(std::vector<char> const&)> forward_;
  std::experimental::net::io_context* io_context_;

  DirectPlayServer dps;
  std::shared_ptr<proxy> host_proxy_;
  std::vector<std::shared_ptr<proxy>> peer_proxies_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_INTERCEPTOR_HPP_
