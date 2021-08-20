#ifndef INCLUDE_DP_PACKEDPLAYER_HPP_
#define INCLUDE_DP_PACKEDPLAYER_HPP_

#include <memory>
#include <vector>
#include <string>

#include "dp/types.h"
#include "experimental/net"

// DPLAYI_PACKEDPLAYER
//
// This structure contains data related to players or groups
#pragma pack(push, 1)
typedef struct {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwPlayerID;
  DWORD dwShortNameLength;  //  Length of the player's shortname
  DWORD dwLongNameLength;   //  Length of the player's longname
  DWORD dwSPDataSize;       //  Size of the service provider field
  DWORD dwPlayerDataSize;
  DWORD dwNumberOfPlayers;
  DWORD dwSysPlayerID;
  DWORD dwFixedSize;  //  = 48
  DWORD dwPlayerVersion;
  DWORD dwParentID;
  BYTE data[];
  //  WSTR  szShortName[];
  //  WSTR  szLongName[];
  //  sockaddr_in addrStream;
  //  sockaddr_in addrDatagram;
  //  BYTE  playerData[];
  //  DWORD playerIDs[];
} DPLAYI_PACKEDPLAYER;
#pragma pack(pop)

namespace dp {
class packedplayer {
 public:
  enum Flags : int {
    SYSTEMPLAYER = 0x1,
    NAMESERVER = 0x2,
    PLAYERINGROUP = 0x4,
    PLAYERSENDINGMACHINE = 0x8
  };

  packedplayer(BYTE* data);

  std::vector<BYTE> to_vector(void);
  std::size_t size(void);

  Flags flags;
  DWORD player_id;
  DWORD system_id;
  DWORD player_version;
  DWORD parent_id;
  std::string short_name;
  std::string long_name;
  std::experimental::net::ip::tcp::endpoint dp_address;
  std::experimental::net::ip::udp::endpoint data_address;
  std::vector<BYTE> player_data;
  std::vector<DWORD> player_ids;

 private:
  // Data Pointer Loaders
  BYTE* get_short_name_ptr(void);
  BYTE* get_long_name_ptr(void);
  BYTE* get_dp_address_ptr(void);
  BYTE* get_data_address_ptr(void);
  BYTE* get_player_data_ptr(void);
  BYTE* get_player_ids_ptr(void);

  // Data Constructors
  std::string load_short_name(void);
  std::string load_long_name(void);
  std::experimental::net::ip::tcp::endpoint load_dp_address(void);
  std::experimental::net::ip::udp::endpoint load_data_address(void);
  std::vector<BYTE> load_player_data(void);
  std::vector<DWORD> load_player_ids(void);

  // Data Assigners
  void assign_short_name(void);
  void assign_long_name(void);
  void assign_dp_address(void);
  void assign_data_address(void);
  void assign_player_data(void);
  void assign_player_ids(void);

  DPLAYI_PACKEDPLAYER *player_;
};
}  // namespace dp

inline dp::packedplayer::Flags operator|(dp::packedplayer::Flags const &lhs,
                                         dp::packedplayer::Flags const &rhs) {
  return static_cast<dp::packedplayer::Flags>(static_cast<int>(lhs) |
                                              static_cast<int>(rhs));
}

#endif  // INCLUDE_DP_PACKEDPLAYER_HPP_
