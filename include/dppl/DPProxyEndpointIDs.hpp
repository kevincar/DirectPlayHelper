#ifndef INCLUDE_DPPL_DPPROXYENDPOINTIDS_HPP_
#define INCLUDE_DPPL_DPPROXYENDPOINTIDS_HPP_

#include "dplay.h"

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
}
#endif  // INCLUDE_DPPL_DPPROXYENDPOINTIDS_HPP_
