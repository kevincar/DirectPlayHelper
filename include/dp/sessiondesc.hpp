#ifndef INCLUDE_DP_SESSIONDESC_HPP_
#define INCLUDE_DP_SESSIONDESC_HPP_

#include <memory>
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

namespace dp {
class sessiondesc {
 public:
  enum Flags : int {
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

  explicit sessiondesc(BYTE* data);

  std::vector<BYTE> to_vector(void);

  Flags flags;
  GUID instance;
  GUID application;
  DWORD max_players;
  DWORD num_players;
  DWORD session_id;
  DWORD reserved2;
  DWORD user1;
  DWORD user2;
  DWORD user3;
  DWORD user4;

 private:
  DPSESSIONDESC2* session_;
};
}  // namespace dp

inline dp::sessiondesc::Flags operator|(dp::sessiondesc::Flags const& lhs,
                                        dp::sessiondesc::Flags const& rhs) {
  return dp::sessiondesc::Flags(static_cast<int>(lhs) | static_cast<int>(rhs));
}

#endif  // INCLUDE_DP_SESSIONDESC_HPP_
