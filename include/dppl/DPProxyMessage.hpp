#ifndef INCLUDE_DPPL_DPPROXYMESSAGE_HPP_
#define INCLUDE_DPPL_DPPROXYMESSAGE_HPP_

#include <algorithm>
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
// A `DPProxyMessage` is a DirectPlay message associated with endpoint
// metadata; i.e., it's simply the binary information for a DPMessage that also
// comes with important information for proxies to know who the message came
// from and who it's intended for
class DPProxyMessage {
 public:
  // The Vector Chars are the binary data, and the first and second endpoints
  // or proxy are the recipient and sender respectively
  template <typename T>
  DPProxyMessage(std::vector<T>, proxy const&, proxy const&);
  template <typename T>
  DPProxyMessage(std::vector<T>, DPProxyEndpointIDs, proxy const&);
  template <typename T>
  DPProxyMessage(std::vector<T>, proxy const&, DPProxyEndpointIDs);
  template <typename T>
  DPProxyMessage(std::vector<T>, DPProxyEndpointIDs, DPProxyEndpointIDs);
  template <typename T>
  explicit DPProxyMessage(std::vector<T>);

  DPProxyMessage(DPProxyMessage const&);

  // Convert the DPProxyMessage into a byte vector
  std::vector<char> to_vector() const;

  // Get the DirectPlay Message as a byte vactor
  std::vector<char> get_dp_msg_data() const;
  // Return a copy of a DPMessage whos internal data structure is a pointer to
  // the same data here. Thus manipulations on the DPMessage structure returned
  // here will affect the data on the DPProxyMessage that returned it
  DPMessage get_dp_msg();

  DPProxyEndpointIDs get_to_ids() const;

  DPProxyEndpointIDs get_from_ids() const;

  void set_to_ids(DPProxyEndpointIDs);
  void set_to_ids(proxy const&);
  void set_from_ids(DPProxyEndpointIDs);
  void set_from_ids(proxy const&);

  // Determine whether the message is a dp message or a data message
  bool is_dp_message(void) const;

  template <typename T>
  static std::vector<char> pack_message(std::vector<T> message_data,
                                        proxy const& to, proxy const& from);
  template <typename T>
  static std::vector<char> pack_message(std::vector<T> message_data,
                                        proxy const& to,
                                        DPProxyEndpointIDs fromIDs);
  template <typename T>
  static std::vector<char> pack_message(std::vector<T> message_data,
                                        DPProxyEndpointIDs toIDs,
                                        proxy const& from);
  template <typename T>
  static std::vector<char> pack_message(std::vector<T> message_data,
                                        DPProxyEndpointIDs toIDs,
                                        DPProxyEndpointIDs fromIDs);

 private:
  static DPProxyEndpointIDs proxy_to_ids(proxy const& p);
  std::vector<char> data_;
};

template <typename T>
DPProxyMessage::DPProxyMessage(std::vector<T> message_data, proxy const& to,
                               proxy const& from)
    : data_(DPProxyMessage::pack_message(message_data, to, from)) {}

template <typename T>
DPProxyMessage::DPProxyMessage(std::vector<T> message_data,
                               DPProxyEndpointIDs toIDs, proxy const& from)
    : data_(DPProxyMessage::pack_message(message_data, toIDs, from)) {}

template <typename T>
DPProxyMessage::DPProxyMessage(std::vector<T> message_data, proxy const& to,
                               DPProxyEndpointIDs fromIDs)
    : data_(DPProxyMessage::pack_message(message_data, to, fromIDs)) {}

template <typename T>
DPProxyMessage::DPProxyMessage(std::vector<T> message_data,
                               DPProxyEndpointIDs toIDs,
                               DPProxyEndpointIDs fromIDs)
    : data_(DPProxyMessage::pack_message(message_data, toIDs, fromIDs)) {}

template <typename T>
DPProxyMessage::DPProxyMessage(std::vector<T> data) : data_(data) {}

template <typename T>
std::vector<char> DPProxyMessage::pack_message(std::vector<T> message_data,
                                               proxy const& to,
                                               proxy const& from) {
  DPProxyEndpointIDs toIDs = DPProxyMessage::proxy_to_ids(to);
  DPProxyEndpointIDs fromIDs = DPProxyMessage::proxy_to_ids(from);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

template <typename T>
std::vector<char> DPProxyMessage::pack_message(std::vector<T> message_data,
                                               proxy const& to,
                                               DPProxyEndpointIDs fromIDs) {
  DPProxyEndpointIDs toIDs = DPProxyMessage::proxy_to_ids(to);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

template <typename T>
std::vector<char> DPProxyMessage::pack_message(std::vector<T> message_data,
                                               DPProxyEndpointIDs toIDs,
                                               proxy const& from) {
  DPProxyEndpointIDs fromIDs = DPProxyMessage::proxy_to_ids(from);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

template <typename T>
std::vector<char> DPProxyMessage::pack_message(std::vector<T> message_data,
                                               DPProxyEndpointIDs toIDs,
                                               DPProxyEndpointIDs fromIDs) {
  std::size_t new_len = sizeof(DPPROXYMSG) + message_data.size();
  std::vector<char> retval(new_len, '\0');
  DPPROXYMSG* imsg = reinterpret_cast<DPPROXYMSG*>(&(*retval.begin()));
  imsg->to.clientID = toIDs.clientID;
  imsg->to.systemID = toIDs.systemID;
  imsg->to.playerID = toIDs.playerID;
  imsg->from.clientID = fromIDs.clientID;
  imsg->from.systemID = fromIDs.systemID;
  imsg->from.playerID = fromIDs.playerID;
  char* dest = reinterpret_cast<char*>(&imsg->dp_message);
  std::copy(message_data.begin(), message_data.end(), dest);
  return retval;
}
}  // namespace dppl
#endif  // INCLUDE_DPPL_DPPROXYMESSAGE_HPP_
