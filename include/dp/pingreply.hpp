#ifndef INCLUDE_DP_PINGREPLY_HPP_
#define INCLUDE_DP_PINGREPLY_HPP_

#include <vector>

#include "dp/base_message.hpp"
#include "dp/types.h"

// DPMSG_PINGREPLY
//
// This packet is sent in response to a DPSP_MSG_PING message
#define DPSYS_PINGREPLY 0x0017
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;     //  The identifier of the player who sent
                      //  the ping for which this is a response.
  DWORD dwTickCount;  //  The value in the DPSP_MSG_PING for
                      //  which this is the reply.
} DPMSG_PINGREPLY;
#pragma pack(pop)

namespace dp {
class pingreply : public base_message {
 public:
  explicit pingreply(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  DWORD from_id;
  DWORD tick_count;

 private:
  BYTE* data_;
};
}  // namespace dp

#endif  // INCLUDE_DP_PINGREPLY_HPP_
