#ifndef INCLUDE_DPPL_INTERCEPTOR_HPP_
#define INCLUDE_DPPL_INTERCEPTOR_HPP_
#include <memory>
#include <vector>

#include "dppl/DirectPlayServer.hpp"
#include "dppl/dplay.h"
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
              std::function<void(std::vector<char> const&)> dp_forward,
              std::function<void(std::vector<char> const&)> data_forward);

  // These `_deliver` functions allow the owning class to send data to the
  // local DirectPlay Application through one of the `interceptor`'s proxies.
  // The proxy through which the message is sent is determined by the id
  // information providd in the structure of the message. The underlying data
  // should be in the format of a DPProxyMessage structure.
  void dp_deliver(std::vector<char> const& buffer);
  void data_deliver(std::vector<char> const& buffer);

  void set_client_id(DWORD id);

 private:
  // Proxy Helper Funcs
  inline bool has_proxies();
  std::shared_ptr<proxy> find_peer_proxy(DWORD const& clientid);
  std::shared_ptr<proxy> find_peer_proxy_by_systemid(DWORD const& id);
  std::shared_ptr<proxy> find_peer_proxy_by_playerid(DWORD const& id);
  bool has_free_peer_proxy();

  //
  std::shared_ptr<proxy> get_free_peer_proxy();
  void direct_play_server_callback(std::vector<char> const& buffer);
  void proxy_dp_callback(DPProxyMessage const& message);
  void proxy_data_callback(DPProxyMessage const& message);

  // handlers for messages from remotes
  void dp_send_enumsessions();
  void dp_send_requestplayerid();
  void dp_send_requestplayerreply();
  void dp_send_createplayer();
  void dp_send_addforwardrequest();
  void dp_send_superenumplayersreply();
  std::size_t register_player(DPLAYI_SUPERPACKEDPLAYER* player);

  // handlers for messages from local
  void dp_recv_requestplayerid();
  void dp_recv_superenumplayersreply();

  // Convenience functions for accesing the buffers as DPProxyMessages
  DPProxyMessage get_send_msg();

  DWORD client_id_ = 0;
  DWORD system_id_ = 0;
  DWORD player_id_ = 0;
  int recent_player_id_flags_ = -1;

  std::vector<char> send_buf_;
  std::vector<char> recv_buf_;
  std::function<void(std::vector<char> const&)> dp_forward_;
  std::function<void(std::vector<char> const&)> data_forward_;
  std::experimental::net::io_context* io_context_;

  DirectPlayServer dps;
  std::shared_ptr<proxy> host_proxy_;
  std::vector<std::shared_ptr<proxy>> peer_proxies_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_INTERCEPTOR_HPP_
