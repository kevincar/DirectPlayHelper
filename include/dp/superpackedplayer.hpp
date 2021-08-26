#ifndef INCLUDE_DP_SUPERPACKEDPLAYER_HPP_
#define INCLUDE_DP_SUPERPACKEDPLAYER_HPP_

#include <string>
#include <vector>

#include "experimental/net"
#include "dp/types.h"
#include "dp/playerinfomask.hpp"

// DPLAYI_SUPERPACKEDPLAYER
//
// This structure is used to transmit player or group-related data.
#pragma pack(push, 1)
typedef struct {
  // The size of the fixed player header = 16
  DWORD dwSize;

  // Player flags. Player Flags MUST be 0 or more of the following values:
  DWORD dwFlags;

  // MUST contain the player ID of the player that is described in this
  // structure.
  DWORD ID;

  DWORD dwPlayerInfoMask;

  // If the DPLAYI_PLAYER_SYSPLAYER flag is set in the Flags field, this field
  // MUST contain the protocol version for the machine hosting the protocol. If
  // the DPLAYI_PLAYER_SYSPLAYER flag is not set, this field MUST contain the ID
  // of the system player for this game.  When the protocol version is used for
  // a system player, it will be one of the following values:
  //   9 = First version documented.
  //   10 = New Hosts send DPMSG_IAMNAMESERVER as first message when they become
  //   the new host. 11 = No Change. 12 = The version in which DirectPlay Voice
  //   was introduced. Does not affect any of the core logic. 13 = Added
  //   DPMSG_CREATEPLAYERVERIFY message. 14 = No Change.
  DWORD dwSystemPlayerID;

  BYTE data[];
  // WSTR  szShortName[];           //  If the SN bit in the PlayerInfoMask
  //  field is set, the ShortName field MUST
  //  contain a null-terminated Unicode
  //  string that contains the short name of
  //  the player.
  // WSTR  szLongName[];            //  If the LN bit in the PlayerInfoMask
  //  field is set, the LongName field MUST
  //  contain a null-terminated Unicode
  //  string that contains the long name of
  //  the player.
  // DWORD dwPlayerDataLength;      //  The PD bits in PlayerInfoMask indicate
  //  the size of this optional field. When
  //  present, this field MUST contain the
  //  size of the PlayerData field.
  // BYTE playerData[];             //  If PlayerDataSize is nonzero, this MUST
  //  be set to per-game player data.
  // DWORD szServiceProviderDataLength;  //  The SL bits in PlayerInfoMask
  //  indicate the size of this optional
  //  field. When present, this field MUST
  //  contain the size, in octets, of the
  //  ServiceProviderData field.
  // struct {                       //  If ServiceProviderDataSize is nonzero,
  //  this MUST be set to the data that is
  //  used by the DirectPlay Service
  //  Provider.  If provided, the Windows
  //  Winsock DirectPlay Service Provider
  //  stores the following data in the
  //  ServiceProviderData field:
  // sockaddr_in streamSock;      //  A SOCKADDR_IN structure that contains
  //  the addressing information to be used
  //  when contacting this player over TCP.
  //  If the PL flag is set in the Flags
  //  field, the Address field of this
  //  SOCKADDR_IN must be set to 0.0.0.0.
  // sockaddr_in dataSock;        //  A SOCKADDR_IN structure that contains
  //  the addressing information to be used
  //  when contacting this player over UDP.
  //  If the PL flag is set in the Flags
  //  field, the Address field of this
  //  SOCKADDR_IN must be set to 0.0.0.0.
  // };
  // DWORD dwPlayerCount;           //  The PC bits in PlayerInfoMask indicate
  //  the size of this optional field. When
  //  present, this field MUST contain the
  //  number of entries in the PlayerIDs
  //  field.
  // DWORD dwPlayerIDs[];           //  If the PlayerCount field is present and
  //  nonzero, this MUST be set to a list of
  //  player IDs that are contained in the
  //  group. The length of this field is
  //  equivalent to the value of the
  //  PlayerCount field multiplied by four.
  // DWORD dwParentID;              //  If the PI field is set in the
  //  PlayerInfoMask, this field MUST be set
  //  to the ID of the parent for this group.
  // DWORD dwShortcutIDCount;       //  The SC bits in PlayerInfoMask indicate
  //  the size of this optional field. When
  //  present, this field MUST contain the
  //  number of shortcut IDs in the
  //  ShortcutIDs field.
  // DWORD dwShortcutIDs[];         //  If the ShortcutIDCount field is
  //  nonzero, this MUST be set to a list of
  //  shortcut IDs. The length of this field
  //  is equivalent to the value of
  //  ShortcutIDCount multiplied by four.
} DPLAYI_SUPERPACKEDPLAYER;
#pragma pack(pop)

namespace dp {
class superpackedplayer {
 public:
  enum class Flags : int {
    // SP (1 bit): The player is the system player.
    issystemplayer = 0x1,

    // NS (1 bit): The player is the name server (host). It MUST be combined
    // with SP.
    isnameserver = 0x2,

    // PG (1 bit): The player belongs to a group. This flag MUST be set for
    // system players, for other players that have been added to a group using
    // DPMSG_ADDPLAYERTOGROUP, or for groups that have been added to a group
    // using DPMSG_ADDSHORTCUTTOGROUP.
    isingroup = 0x4,

    // PL (1 bit): The player is on the sending machine.  This flag does not
    // have meaning on other machines and MUST be ignored on receipt.
    islocalplayer = 0x8
    // X (28 bits): All bits that have this label SHOULD be set to zero when
    // sent and MUST be ignored on receipt.
  };

  explicit superpackedplayer(BYTE* data);
  std::vector<BYTE> to_vector(void);
  std::size_t size(void);
  bool is_group(void);

  Flags flags;
  DWORD id;
  playerinfomask mask;
  DWORD system_id;
  std::string short_name;
  std::string long_name;
  std::size_t player_data_size;
  std::vector<BYTE> player_data;
  std::experimental::net::ip::tcp::endpoint dp_address;
  std::experimental::net::ip::udp::endpoint data_address;
  std::size_t num_players;
  std::vector<DWORD> player_ids;
  DWORD parent_id;
  std::size_t num_shortcut_ids;
  std::vector<DWORD> shortcut_ids;

 private:
  // Pointers
  BYTE* get_short_name_ptr(void);
  BYTE* get_long_name_ptr(void);
  BYTE* get_player_data_size_ptr(void);
  BYTE* get_player_data_ptr(void);
  BYTE* get_address_size_ptr(void);
  BYTE* get_dp_address_ptr(void);
  BYTE* get_data_address_ptr(void);
  BYTE* get_num_players_ptr(void);
  BYTE* get_player_ids_ptr(void);
  BYTE* get_parent_id_ptr(void);
  BYTE* get_num_shortcut_ids_ptr(void);
  BYTE* get_shortcut_ids_ptr(void);

  // Loaders
  void load_short_name(void);
  void load_long_name(void);
  void load_player_data_size(void);
  void load_player_data(void);
  void load_dp_address(void);
  void load_data_address(void);
  void load_num_players(void);
  void load_player_ids(void);
  void load_parent_id(void);
  void load_num_shortcut_ids(void);
  void load_shortcut_ids(void);

  // Assigners
  void assign_short_name(void);
  void assign_long_name(void);
  void assign_player_data_size(void);
  void assign_player_data(void);
  void assign_address_size(void);
  void assign_dp_address(void);
  void assign_data_address(void);
  void assign_num_players(void);
  void assign_player_ids(void);
  void assign_parent_id(void);
  void assign_num_shortcut_ids(void);
  void assign_shortcut_ids(void);

  void adjust_mask(void);
  std::size_t convert_size_code(DWORD size_code);

  DPLAYI_SUPERPACKEDPLAYER* player_;
};

inline constexpr superpackedplayer::Flags operator|(
    superpackedplayer::Flags const& lhs, superpackedplayer::Flags const& rhs) {
  return static_cast<superpackedplayer::Flags>(static_cast<int>(lhs) |
                                               static_cast<int>(rhs));
}

}  // namespace dp

#endif  // INCLUDE_DP_SUPERPACKEDPLAYER_HPP_
