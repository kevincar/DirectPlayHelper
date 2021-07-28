#ifndef INCLUDE_DPPL_DPPROXYMESSAGE_HPP_
#define INCLUDE_DPPL_DPPROXYMESSAGE_HPP_

#include <vector>

#include "dppl/dplay.h"
#include "dppl/proxy.hpp"
namespace dppl {
#pragma pack(push, 1)
typedef struct {
  DWORD clientID;
  DWORD systemID;
  DWORD playerID;
} DPProxyEndpointIDs;
typedef struct {
  DPProxyEndpointIDs to;
  DPProxyEndpointIDs from;
  BYTE dp_message[];
} DPPROXYMSG;
#pragma pack(pop)

class proxy;
class DPProxyMessage {
 public:
  DPProxyMessage(std::vector<char>, proxy const&, proxy const&);
  DPProxyMessage(std::vector<char>, DPProxyEndpointIDs, proxy const&);
  DPProxyMessage(std::vector<char>, proxy const&, DPProxyEndpointIDs);
  DPProxyMessage(std::vector<char>, DPProxyEndpointIDs, DPProxyEndpointIDs);
  explicit DPProxyMessage(std::vector<char>);

  DPProxyMessage(DPProxyMessage const&);

  std::vector<char> to_vector() const;

  std::vector<char> get_dp_msg() const;

  DPProxyEndpointIDs get_to_ids() const;

  DPProxyEndpointIDs get_from_ids() const;

  static std::vector<char> pack_message(std::vector<char> message_data,
                                        proxy const& to, proxy const& from);
  static std::vector<char> pack_message(std::vector<char> message_data,
                                        proxy const& to,
                                        DPProxyEndpointIDs fromIDs);
  static std::vector<char> pack_message(std::vector<char> message_data,
                                        DPProxyEndpointIDs toIDs,
                                        proxy const& from);
  static std::vector<char> pack_message(std::vector<char> message_data,
                                        DPProxyEndpointIDs toIDs,
                                        DPProxyEndpointIDs fromIDs);

 private:
  static DPProxyEndpointIDs proxy_to_ids(proxy const& p);
  std::vector<char> data_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_DPPROXYMESSAGE_HPP_
