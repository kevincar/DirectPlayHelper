#ifndef INCLUDE_DP_REQUESTPLAYERID_HPP_
#define INCLUDE_DP_REQUESTPLAYERID_HPP_

#include <vector>

#include "dp/types.h"

// DPMSG_REQUESTPLAYERID
//
// This packet is sent to the host to request a new player ID Two of these
// messages are often sent to optain two different IDs the Player ID
// (dwFlags=8) and the System Player ID  (dwFlags=9)
#define DPSYS_REQUESTPLAYERID 0x0005
#pragma pack(push, 1)
typedef struct {
  DWORD dwFlags;
} DPMSG_REQUESTPLAYERID;
#pragma pack(pop)

namespace dp {
class requestplayerid {
 public:
  enum class Flags : DWORD {
    issystemplayer = 0x1,
    isnameserver = 0x2,
    islocalplayer = 0x4,
    unknown = 0x8,
    issecuresession = 0x200
  };

  explicit requestplayerid(BYTE* data);
  std::vector<BYTE> to_vector(void);

  Flags flags;

 private:
  DPMSG_REQUESTPLAYERID* message_;
};
inline requestplayerid::Flags operator|(requestplayerid::Flags const& lhs,
                                        requestplayerid::Flags const& rhs) {
  return requestplayerid::Flags(static_cast<DWORD>(lhs) |
                                static_cast<DWORD>(rhs));
}
}  // namespace dp

#endif  // INCLUDE_DP_REQUESTPLAYERID_HPP_
