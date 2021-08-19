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

namespace dp {
  class sessiondesc {
    public:
      sessiondesc(std::vector<BYTE> *message_data, BYTE *data);

    private:
      std::vector<BYTE> *message_data_;
      BYTE *data_;
      DPSESSIONDESC2 *session_;
  };
}

#endif INCLUDE_DP_SESSIONDESC_HPP_
