#ifndef INCLUDE_DP_SUPERENUMPLAYERSREPLY_HPP_
#define INCLUDE_DP_SUPERENUMPLAYERSREPLY_HPP_

#include <string>
#include <vector>

#include "dp/types.h"
#include "dp/sessiondesc.hpp"
#include "dp/superpackedplayer.hpp"

// DPMSG_SUPERENUMPLAYERSREPLY
//
// This packet can be sent in response to a DPMSG_ENUMPLAYER message or a
// DPMSG_ADDFORWARDREQUEST message.
#define DPSYS_SUPERENUMPLAYERSREPLY 0x0029
#pragma pack(push, 1)
typedef struct {
  DWORD dwPlayerCount;           //  Number of players.
  DWORD dwGroupCount;            //  Number of groups.
  DWORD dwPackedOffset;          //  Offset of the SuperPackedPlayer field
  DWORD dwShortcutCount;         //  Number of groups with shortcuts.
  DWORD dwDescriptionOffset;     //  The offset of the DPSessionDesc field
  DWORD dwNameOffset;            //  The offset to the SessionName field. A
                                 //  value of zero means a null session
                                 //  name.
  DWORD dwPasswordOffset;        //  The offset of the Password field
  DPSESSIONDESC2 dpSessionDesc;  //  MUST be set to a DPSESSIONDESC2 structure
  // WSTR  szSessionName[];         //  Null-terminated Unicode string that
  //  contains the session name.
  // WSTR  szPassword[];            //  Null-terminated Unicode string that
  //  contains the password for the session.
  // DPLAYI_SUPERPACKEDPLAYER players[]; //  Array of DPLAYI_SUPERPACKEDPLAYER
  //  structures. The number of elements in
  //  the array is determined by finding
  //  the sum of the PlayerCount,
  //  GroupCount, and ShortcutCount fields.
  //  The order of items in the array is
  //  fixed, and is as follows: players,
  //  groups, and shortcuts.
} DPMSG_SUPERENUMPLAYERSREPLY;
#pragma pack(pop)

namespace dp {
class superenumplayersreply {
 public:
  explicit superenumplayersreply(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  sessiondesc session_desc;
  std::string session_name;
  std::string password;
  std::vector<superpackedplayer> players;
  std::vector<superpackedplayer> groups;
  std::vector<superpackedplayer> shortcuts;

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

  DPMSG_SUPERENUMPLAYERSREPLY* message_;
};
}  // namespace dp

#endif  // INCLUDE_DP_SUPERENUMPLAYERSREPLY_HPP_
