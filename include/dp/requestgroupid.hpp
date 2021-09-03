#ifndef INCLUDE_DP_REQUESTGROUPID_HPP_
#define INCLUDE_DP_REQUESTGROUPID_HPP_

#include <vector>

#include "dp/types.h"

// DPMSG_REQUESTGROUPID
//
// This packet is sent to the host to request a new group identifier.
#define DPSYS_REQUESTGROUPID 0x0006
#pragma pack(push, 1)
typedef struct {
  DWORD dwFlags;
} DPMSG_REQUESTGROUPID;
#pragma pack(pop)

namespace dp {
class requestgroupid {
 public:
  enum class Flags : int { islocalgroup = 8 };
  explicit requestgroupid(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  Flags flags;

 private:
  DPMSG_REQUESTGROUPID* message_;
};
};  // namespace dp

#endif  // INCLUDE_DP_REQUESTGROUPID_HPP_
