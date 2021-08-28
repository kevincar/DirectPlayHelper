#ifndef INCLUDE_DPPL_ENDPOINTIDS_HPP_
#define INCLUDE_DPPL_ENDPOINTIDS_HPP_

#include <string>

#include "dp/types.h"

namespace dppl {
#pragma pack(push, 1)
typedef struct {
  DWORD clientID;
  DWORD systemID;
  DWORD playerID;
} ENDPOINTIDS;
typedef struct {
  ENDPOINTIDS to;
  ENDPOINTIDS from;
  BYTE message[];
} PROXYMSG;
#pragma pack(pop)
inline std::string endpoint_to_str(ENDPOINTIDS const& rhs) {
  return std::string("clientID: ") + std::to_string(rhs.clientID) +
         ", systemID: " + std::to_string(rhs.systemID) +
         ", playerID: " + std::to_string(rhs.playerID);
}
}  // namespace dppl
#endif  // INCLUDE_DPPL_DPPROXYENDPOINTIDS_HPP_
