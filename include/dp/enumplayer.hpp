#ifndef INCLUDE_DP_ENUMPLAYER_HPP_
#define INCLUDE_DP_ENUMPLAYER_HPP_

#include "dp/types.h"

// DPMSG_DPSP_MSG_ENUMPLAYER
//
// This packet is sent to the server to request an enumeration of players
#define DPSYS_ENUMPLAYER 0x0004
#pragma pack(push, 1)
typedef struct {
} DPMSG_ENUMPLAYER;
#pragma pack(pop)

#endif  // INCLUDE_DP_ENUMPLAYER_HPP_
