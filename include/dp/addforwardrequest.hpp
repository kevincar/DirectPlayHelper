#ifndef INCLUDE_DP_ADDFORWARDREQUEST_HPP_
#define INCLUDE_DP_ADDFORWARDREQUEST_HPP_

#include <string>
#include <vector>

#include "dp/base_message.hpp"
#include "dp/packedplayer.hpp"
#include "dp/types.h"

// DPMSG_ADDFORWARDREQUEST
//
// This packet is sent to forward a message to a downstream player.
#define DPSYS_ADDFORWARDREQUEST 0x0013
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  Identifier of the player to whom the
                           //  message is being sent.
  DWORD dwPlayerID;        //  The identity of the player being added.
  DWORD dwGroupID;         //  SHOULD be set to zero
  DWORD dwCreateOffset;    //  Offset of the PlayerInfo field
  DWORD dwPasswordOffset;  //  Offset of the Password field
  BYTE data[];
  // DPLAYI_PACKEDPLAYER dpPlayer;  //  Information about the system player on
  //  the newly added machine.
  // WSTR  szPassword[];            //  Null-terminated Unicode string that
  //  contains the session password.
  // DWORD dwTickCount;             //  The computing system tick count when
  //  the packet was generated.
} DPMSG_ADDFORWARDREQUEST;
#pragma pack(pop)

namespace dp {
class addforwardrequest : public base_message {
 public:
  explicit addforwardrequest(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  DWORD to_id;
  DWORD id;
  DWORD group_id;
  packedplayer player;
  std::string password;
  DWORD tick_count;

 private:
  // Pointers
  BYTE* get_player_ptr(void);
  BYTE* get_password_ptr(void);
  BYTE* get_tick_count_ptr(void);

  // Loaders
  void load_password(void);
  void load_tick_count(void);

  // Assigners
  void assign_player(void);
  void assign_password(void);
  void assign_tick_count(void);

  DPMSG_ADDFORWARDREQUEST* message_;
};
}  // namespace dp

#endif  // INCLUDE_DP_ADDFORWARDREQUEST_HPP_
