#ifndef INCLUDE_DP_CREATEPLAYER_HPP_
#define INCLUDE_DP_CREATEPLAYER_HPP_
#include <string>
#include <vector>

#include "dp/packedplayer.hpp"
#include "dp/types.h"

// DPMSG_CREATEPLAYER
//
// This packet is sent to indicate that a new player has been created.
#define DPSYS_CREATEPLAYER 0x0008
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;          //  ID Player to whom the
                         //  message is being sent.
  DWORD dwID;            //  ID Player from where it was sent
  DWORD dwGroupID;       //  GroupID (4 bytes): Ignored
  DWORD dwCreateOffset;  //  Offset of the PlayerInfo field. = 28
  DWORD dwPasswordOffset;
  BYTE data[];
  //  DPLAYI_PACKEDPLAYER dpPlayerInfo;
  //  WORD  cbReserved1;
  //  DWORD dwReserved2;
} DPMSG_CREATEPLAYER;
#pragma pack(pop)

namespace dp {
class createplayer {
 public:
  explicit createplayer(BYTE* data);
  std::vector<BYTE> to_vector(void);

  DWORD to_id;
  DWORD id;
  DWORD group_id;
  packedplayer player;
  WORD reserved1;
  DWORD reserved2;

 private:
  // Pointers
  BYTE* get_player_ptr(void);
  BYTE* get_reserved1_ptr(void);
  BYTE* get_reserved2_ptr(void);

  // Loaders
  void load_reserved1(void);
  void load_reserved2(void);

  // Assigners
  void assign_player(void);
  void assign_reserved1(void);
  void assign_reserved2(void);

  DPMSG_CREATEPLAYER* message_;
};
}  // namespace dp

#endif  // INCLUDE_DP_CREATEPLAYER_HPP_
