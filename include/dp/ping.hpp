#ifndef INCLUDE_DP_PING_HPP_
#define INCLUDE_DP_PING_HPP_

#include <vector>

#include "dp/base_message.hpp"
#include "dp/types.h"

// DPMSG_PING
//
// This packet is used to keep the UDP session active and to optimize the
// protocol.

#define DPSYS_PING 0x0016
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;     //  The identifier of the player who sent
                      //  the ping.
  DWORD dwTickCount;  //  The number of milliseconds that have
                      //  elapsed since the computer system was started.
} DPMSG_PING;
#pragma pack(pop)

namespace dp {
class ping : public base_message {
 public:
  explicit ping(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  DWORD from_id;
  DWORD tick_count;

 private:
  BYTE* data_;
};
}  // namespace dp

#endif  // INCLUDE_DP_PING_HPP_
