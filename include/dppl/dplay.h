
#ifndef DPPL_DPLAY_H_
#define DPPL_DPLAY_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
typedef unsigned long   QWORD;

typedef unsigned char   STR, *LPSTR;
typedef char16_t        WSTR, *LPWSTR;
#endif  // define(WIN32)

/*
 * GUID
 * Global Unique Identifier
 */
#pragma pack(push, 1)
typedef struct {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8]; 
} GUID;
#pragma pack(pop)

namespace dplay {
/*
 * DPMSG_HEADER
 *
 * The DPMSG_HEADER is prepended to all DirectPlay Protocol messages and
 * contains an identifier that describes each message structure.
 */
#pragma pack(push, 1)
typedef struct {
  WORD        cbSize;               //  Indicates the size of the message
  WORD        token;                //  Describes high-level message
                                    //  characteristics:
                                    //    0xFAB = Indicates that the message
                                    //            was received from a remote
                                    //            DirectPlay machine.
                                    //    0xCAB = Indicates that the message
                                    //            will be forwarded to all
                                    //            registered servers.
                                    //    0xBAB = Indicates that the message
                                    //            was received from a
                                    //            DirectPlay server.
  sockaddr_in saddr;                //  16 bytes of data containing the sockets
  STR         signature[4];         //  MUST be set to the value 0x79616c70
                                    //  (ASCII 'play')
  WORD        command;              //  For messages below
  WORD        version;              //  MUST be set to the version number of
                                    //  the protocol
} DPMSG_HEADER;
#pragma pack(pop)

/*
 * DPSESSIONDESC2
 * Used to describe the properties of a DirectPlay
 * session instance
 */
#pragma pack(push, 1)
typedef struct {
  DWORD dwSize;                     //  Size of structure
  DWORD dwFlags;                    //  DPSESSION_xxx flags
  GUID  guidInstance;               //  ID for the session instance
  GUID  guidApplication;            //  GUID of the DirectPlay Application
                                    //  GUID_NULL for all applications
  DWORD dwMaxPlayers;               //  Maximum # players allowed in session
  DWORD dwCurrentPlayers;           //  Current # players in session (read only)
  union {                           //  Name of the session
    DWORD lpszSessionName;          //  Unicode
    DWORD lpszSessionNameA;         //  ANSI
  };
  union {                           //  Password of the session (optional)
    DWORD lpszPassword;             //  Unicode
    DWORD lpszPasswordA;            //  ANSI
  };
  DWORD dpSessionID;                //  Session ID
  DWORD dwReserved2;                //  Reserved
  DWORD dwUser1;                    //  For application use
  DWORD dwUser2;
  DWORD dwUser3;
  DWORD dwMaxRank;
} DPSESSIONDESC2;
#pragma pack(pop)

/*****************************************************************************
 *
 * DirectPlay System Messages
 *
 ****************************************************************************/

/*
 * DPMSG_ENUMSESSIONSREPLY
 *
 * This packet is sent by the server in response to a DPSP_MSG_ENUMSESSIONS
 * request
 */
#define DPSYS_ENUMSESSIONSREPLY 0x0001
#pragma pack(push, 1)
typedef struct {
  DPSESSIONDESC2  dpSessionInfo;    //  Session info 
  DWORD           dwNameOffset;     //  offsets, in octets from the beginning
                                    //  of the message in dpSessionInfo to the
                                    //  beginning of the message in
                                    //  szSessionName
  WSTR            szSessionName[];  //  Name of the session

} DPMSG_ENUMSESSIONSREPLY;
#pragma pack(pop)
}

/*
 * DPMSG_ENUMSESSIONS
 *
 * This packet is sent by the client to request an enumeration of DirectPlay
 * sessions
 */
#define DPSYS_ENUMSESSIONS  0x0002
typedef struct {
  GUID  guidApplication;            //  MUST be set to the application
                                    //  identifier for the game.
  DWORD lpPasswordOffset;           //  MUST be set to the offset, in octets,
                                    //  of the password from the beginning of
                                    //  the message.
  DWORD dwFlags;                    //  AV, AL, X, PR, Y
  WSTR  szPassword[];               //  a null-terminated Unicode string that
                                    //  contains the password.
} DPMSG_ENUMSESSIONS;

#endif  // DPPL_DPLAY_H_
