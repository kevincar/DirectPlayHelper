#ifndef INCLUDE_DP_SESSIONDESC_HPP_
#define INCLUDE_DP_SESSIONDESC_HPP_

#include <vector>

#include "dp/types.h"

// DPSESSIONDESC2
//
// Used to describe the properties of a DirectPlay
// session instance
#pragma pack(push, 1)
typedef struct {
  DWORD dwSize;              //  Size of structure
  DWORD dwFlags;             //  DPSESSION_xxx flags
  GUID guidInstance;         //  ID for the session instance
  GUID guidApplication;      //  GUID of the DirectPlay Application
                             //  GUID_NULL for all applications
  DWORD dwMaxPlayers;        //  Maximum # players allowed in session
  DWORD dwCurrentPlayers;    //  Current # players in session (read only)
  union {                    //  Name of the session
    DWORD lpszSessionName;   //  Unicode
    DWORD lpszSessionNameA;  //  ANSI
  };
  union {                 //  Password of the session (optional)
    DWORD lpszPassword;   //  Unicode
    DWORD lpszPasswordA;  //  ANSI
  };
  DWORD dpSessionID;  //  Session ID
  DWORD dwReserved2;  //  Reserved
  DWORD dwUser1;      //  For application use
  DWORD dwUser2;
  DWORD dwUser3;
  DWORD dwUser4;  // Max Rank
} DPSESSIONDESC2;
#pragma pack(pop)

enum DPSESSIONDESCFLAGS {
  nonewplayers = 0x1,
  migratehost = 0x4,
  shortplayermessage = 0x8,
  canjoin = 0x20,
  useping = 0x40,
  noplayerupdates = 0x80,
  useauthentication = 0x100,
  privatesession = 0x200,
  passwordrequired = 0x400,
  routeviagamehost = 0x800,
  getserverplayeronly = 0x1000,
  usereliableprotocol = 0x2000,
  preserveorder = 0x4000,
  optimizeforlatency = 0x8000,
  acquirevoice = 0x10000,
  nosessiondescchanges = 0x20000
};

inline DPSESSIONDESCFLAGS operator|(DPSESSIONDESCFLAGS const &lhs,
                                    DPSESSIONDESCFLAGS const &rhs) {
  return static_cast<DPSESSIONDESCFLAGS>(static_cast<int>(lhs) |
                                         static_cast<int>(rhs));
}

namespace dp {
class sessiondesc {
 public:
  sessiondesc(std::shared_ptr<std::vector<BYTE>> message_data, BYTE *data);

  DWORD get_size(void);
  void set_size(DWORD size);

  DPSESSIONDESCFLAGS get_flags(void);
  void set_flags(DPSESSIONDESCFLAGS flags);

  GUID get_guid_instance(void);
  void set_guid_instance(GUID guid);

  GUID get_guid_application(void);
  void set_guid_application(GUID guid);

  DWORD get_max_players(void);
  void set_max_players(DWORD max_players);

  DWORD get_num_current_players(void);
  void set_num_current_players(DWORD current_players);

  // Skipping session name and password since documention requires that these
  // be NULL

  DWORD get_session_id(void);
  void set_session_id(DWORD session_id);

  DWORD get_reserved_2(void);
  void set_reserved_2(DWORD reserved_value);

  DWORD get_user_1(void);
  void set_user_1(DWORD user_1_value);

  DWORD get_user_2(void);
  void set_user_2(DWORD user_2_value);

  DWORD get_user_3(void);
  void set_user_3(DWORD user_3_value);

  DWORD get_user_4(void);
  void set_user_4(DWORD user_4_value);

 private:
  std::shared_ptr<std::vector<BYTE>> message_data_;
  BYTE *data_;
  DPSESSIONDESC2 *session_;
};
}  // namespace dp

#endif  // INCLUDE_DP_SESSIONDESC_HPP_
