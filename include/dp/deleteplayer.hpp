#ifndef INCLUDE_DP_DELETEPLAYER_HPP_
#define INCLUDE_DP_DELETEPLAYER_HPP_

#include <vector>

#include "dp/base_message.hpp"
#include "dp/types.h"

// DPMSG_DELETEPLAYER
//
// This packet is sent to indicate that a player has been deleted.
#define DPSYS_DELETEPLAYER 0x000B
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  To the player ID of the newly deleted
                           //  player.
  DWORD dwGroupID;         //  SHOULD be set to zero
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_DELETEPLAYER;
#pragma pack(pop)

namespace dp {
class deleteplayer : public base_message {
 public:
  explicit deleteplayer(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  DWORD player_id;

 private:
  DPMSG_DELETEPLAYER* message_;
};
}  // namespace dp
#endif  // INCLUDE_DP_DELETEPLAYER_HPP_
