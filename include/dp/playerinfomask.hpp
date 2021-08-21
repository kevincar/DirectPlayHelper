#ifndef INCLUDE_DP_PLAYERINFOMASK_HPP_
#define INCLUDE_DP_PLAYERINFOMASK_HPP_

#include "dp/types.h"

// PLAYERINFOMASK
//
// A bit field that indicates which optional fields are present. The
// PlayerInfoMask field MUST be a bitmask that is composed of the following
// fields
#pragma pack(push, 1)
typedef struct {
  // SN (1 bit): MUST be set if the ShortName field is present in the structure.
  bool shortNamePresent : 1;

  // LN (1 bit): MUST be set if the LongName field is present in the structure.
  bool longNamePresent : 1;

  // SL (2 bits): MUST be set if the ServiceProviderDataLength field is
  // present in the structure. SL MUST be set to one of the following values.
  //   0x01 = If the ServiceProviderDataLength field occupies 1 byte.
  //   0x02 = If the ServiceProviderDataLength field occupies 2 bytes.
  //   0x03 = If the ServiceProviderDataLength field occupies 4 bytes.
  char serviceProviderLength : 2;

  // PD (2 bits): MUST be set if the PlayerDataLength field is present in the
  // structure. PD MUST be set to one of the following values:
  //   0x01 = If the PlayerDataLength field occupies 1 byte.
  //   0x02 = If the PlayerDataLength field occupies 2 bytes.
  //   0x03 = If the PlayerDataLength field occupies 4 bytes.
  char playerDataLength : 2;

  // PC (2 bits): MUST be set if the PlayerCount field is present in the
  // structure. PC MUST be set to one of the following values:
  //   0x01 = If the PlayerCount field occupies 1 byte.
  //   0x02 = If the PlayerCount field occupies 2 bytes.
  //   0x03 = If the PlayerCount field occupies 4 bytes.
  char playerCountLength : 2;

  // PI (1 bit): MUST be set if the ParentID field is present in the structure.
  bool parentIDPresent : 1;

  // SC (2 bits): MUST be set if the ShortcutCount field is present in the
  // structure. SC MUST be set to one of the following values:
  //   0x01 = If the ShortcutCount field occupies 1 byte.
  //   0x02 = If the ShortcutCount field occupies 2 bytes.
  //   0x03 = If the ShortcutCount field occupies 4 bytes.
  char shortcutCountLength : 2;

  // X (21 bits): All bits with this label SHOULD be set to zero when sent and/
  // MUST be ignored on receipt.
  int x : 21;
} DPPLAYERINFOMASK;
#pragma pack(pop)

namespace dp {
class playerinfomask {
 public:
  explicit playerinfomask(BYTE* data);
  DWORD to_dword(void);

  bool short_name_present;
  bool long_name_present;
  int n_service_provider_size_bytes;
  int n_player_data_size_bytes;
  int n_player_count_size_bytes;
  bool parent_id_present;
  int n_shortcut_count_size_bytes;

 private:
  DWORD value_;
};
}  // namespace dp
#endif  // INCLUDE_DP_PLAYERINFOMASK_HPP_
