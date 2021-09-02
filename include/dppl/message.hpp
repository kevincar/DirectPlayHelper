#ifndef INCLUDE_DPPL_MESSAGE_HPP_
#define INCLUDE_DPPL_MESSAGE_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include "dp/dp.hpp"
#include "dppl/EndpointIDs.hpp"

namespace dppl {

class proxy;
// A `dppl::message` is a DirectPlay transmission associated with endpoint
// metadata; i.e., it's simply the binary information for a transmission that
// also comes with important information for proxies to know who the message
// came from and who it's intended for
class message {
 public:
  message(void);
  message(dp::transmission, ENDPOINTIDS, ENDPOINTIDS);
  explicit message(std::vector<BYTE> data);

  std::size_t size(void) const;
  std::vector<BYTE> to_vector() const;

  ENDPOINTIDS to;
  ENDPOINTIDS from;
  dp::transmission data;
};

}  // namespace dppl
#endif  // INCLUDE_DPPL_MESSAGE_HPP_
