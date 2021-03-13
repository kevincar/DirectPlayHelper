#ifndef INCLUDE_DPPL_INTERCEPTOR_HPP_
#define INCLUDE_DPPL_INTERCEPTOR_HPP_
#include <experimental/net>
#include <set>

#include "dppl/dplay.h"
#include "dppl/probe/host.hpp"
#include "dppl/probe/join.hpp"
#include "dppl/proxy.hpp"
namespace dppl {
class interceptor {
 public:
  enum state { None, Hosting, Joining };

  interceptor(std::experimental::net::io_context* io_context, GUID app_guid,
              std::function<void(std::vector<char>)> forward);

 private:
  void start();

  inline bool has_proxies();
  void analyze_app_state();
  void host_handler(bool hosting);
  void join_handler(bool joining);

  GUID app_guid_;
  bool hosting_ = false;
  bool joining_ = false;
  state app_state_ = state::None;
  std::function<void(std::vector<char>)> forward_;
  std::experimental::net::io_context* io_context_;

  probe::host host_probe_;
  probe::join join_probe_;

  std::shared_ptr<proxy> host_proxy_;
  std::set<std::shared_ptr<proxy>> peer_proxies_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_INTERCEPTOR_HPP_
