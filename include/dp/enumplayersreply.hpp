#ifndef INCLUDE_DP_ENUMPLAYERSREPLY_HPP_
#define INCLUDE_DP_ENUMPLAYERSREPLY_HPP_

#include <string>
#include <vector>

#include "dp/packedplayer.hpp"
#include "dp/sessiondesc.hpp"
#include "dp/types.h"

// DPMSG__ENUMPLAYERSREPLY
//
// This packet can be sent in response to a DPMSG_ENUMPLAYER message or a
// DPMSG_ADDFORWARDREQUEST message
#define DPSYS_ENUMPLAYERSREPLY 0x0003
#pragma pack(push, 1)
typedef struct {
  DWORD dwPlayerCount;           //  number of players
  DWORD dwGroupCount;            //  number of groups
  DWORD dwPlayerOffset;          //  the offset of the PlayerInfo field
  DWORD dwShortcutCount;         //  MUST be ignored on receipt
  DWORD dwDescriptionOffset;     //  the offset of the SessionDesc field
  DWORD dwNameOffset;            //  The offset to the SessionName field
  DWORD dwPasswordOffset;        //  The offset to the Password field
  DPSESSIONDESC2 dpSessionDesc;  //  Session description information
  BYTE data[];
  //  WSTR            szSessionName[];      //  Name for the session
  //  WSTR            szPassword[];         //  Password
  //  DPLAYI_PACKEDPLAYER playerInfo[];
} DPMSG_ENUMPLAYERSREPLY;
#pragma pack(pop)

namespace dp {
class enumplayersreply {
 public:
  explicit enumplayersreply(BYTE* data);
  std::vector<BYTE> to_vector(void);

  DWORD num_players;
  DWORD num_groups;
  DWORD num_shortcuts;
  sessiondesc session_desc;
  std::string session_name;
  std::string password;
  std::vector<packedplayer> players;

 private:
  // Pointers
  BYTE* get_session_name_ptr(void);
  BYTE* get_password_ptr(void);
  BYTE* get_players_ptr(void);

  // Loaders
  void load_session_name(void);
  void load_password(void);
  void load_players(void);

  // Assigners
  void assign_session_name(void);
  void assign_password(void);
  void assign_players(void);

  DPMSG_ENUMPLAYERSREPLY* message_;
};
}  // namespace dp

#endif  // INCLUDE_DP_ENUMPLAYERSREPLY_HPP_
