#ifndef INCLUDE_DPPL_INTERCEPTOR_HPP_
#define INCLUDE_DPPL_INTERCEPTOR_HPP_
#include <memory>
#include <vector>

#include "dp/dp.hpp"
#include "dppl/DirectPlayServer.hpp"
#include "dppl/proxy.hpp"
#include "experimental/net"

#define IOLOG(X) LOG(X) << TXCY
#define IILOG(X) LOG(X) << TXCG
#define IELOG TXRS

namespace dppl {
// The `interceptor` class manages all process that are responsible for
// "intercepting" data and messages sent out by the local DirectPlay
// Application. From a high level, the `interceptor` is responsible for
// managing all proxyies and DirectPlayServer sockets associated with a given
// DirectPlay Application. The `interceptor` also acts in a way like a proxy
// for the local player application to keep track of the information pertaining
// to the user on the application such as the DirectPlay ID for the user, since
// the proxies are doing this for all the other connected peers, etc.
class interceptor {
 public:
  // the `pd_forward` and `data_forward` are callback methods defined by the
  // class that owns the `interceptor`. Messages from the interceptor's
  // `proxy`s and `DirectPlayServer` will be bundled into DPProxyMessages and
  // forwarded through these callbacks.
  interceptor(std::experimental::net::io_context* io_context,
              std::function<void(dppl::message const&)> dp_forward,
              std::function<void(dppl::message const&)> data_forward,
              bool use_localhost = false);

  // These `_deliver` functions allow the owning class to send data to the
  // local DirectPlay Application through one of the `interceptor`'s proxies.
  // The proxy through which the message is sent is determined by the id
  // information providd in the structure of the message. The underlying data
  // should be in the format of a DPProxyMessage structure.
  void dp_deliver(dppl::message const& request);
  void data_deliver(dppl::message const& request);

  // void set_client_id(DWORD id);

 private:
  // Proxy Helper Funcs
  // inline bool has_proxies();
  std::shared_ptr<proxy> find_proxy(ENDPOINTIDS const& ids);
  std::shared_ptr<proxy> add_proxy(proxy::type type);

  // Convenience functions for accesing the buffers as DPProxyMessages
  // DPProxyMessage get_send_msg();

  // For create proxies for other players in the session
  // std::size_t register_player(DPLAYI_SUPERPACKEDPLAYER* player);

  // handlers for messages from remotes
  void dp_send_enumsessions(dppl::message const& request);
  void dp_send_enumsessionsreply(dppl::message const& request);
  void dp_send_requestplayerid(dppl::message const& request);
  // void dp_send_requestplayerreply();
  void dp_send_createplayer(dppl::message const& request);
  void dp_send_addforwardrequest(dppl::message const& request);
  // void dp_send_superenumplayersreply();

  // handlers for messages from local
  // void dp_recv_requestplayerid();
  void dp_recv_superenumplayersreply(dppl::message const& response);

  // Proxy Callbacks
  void proxy_dp_callback(dppl::message message);
  void proxy_data_callback(dppl::message message);

  // DPS Callback to receive ENUMSESSIONS requests
  void direct_play_server_callback(dp::transmission request);

  DWORD client_id_ = 0;
  DWORD system_id_ = 0;
  DWORD player_id_ = 0;
  DWORD recent_player_id_flags_ = 0;

  std::vector<char> send_buf_;
  std::vector<char> recv_buf_;
  std::function<void(dppl::message const&)> dp_forward_;
  std::function<void(dppl::message const&)> data_forward_;
  std::experimental::net::io_context* io_context_;

  DirectPlayServer dps;
  std::experimental::net::ip::tcp::endpoint dps_return_addr_;
  std::vector<std::shared_ptr<proxy>> proxies_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_INTERCEPTOR_HPP_
