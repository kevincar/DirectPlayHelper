
#ifndef INCLUDE_DPPL_DPLAY_H_
#define INCLUDE_DPPL_DPLAY_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <winsock2.h>
#include <windows.h>
typedef unsigned char STR;
typedef char16_t WSTR;
#else
#include <netinet/in.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef unsigned char STR, *LPSTR;
typedef char16_t WSTR, *LPWSTR;

/*
 * GUID
 * Global Unique Identifier
 */
#pragma pack(push, 1)
typedef struct {
  DWORD Data1;
  WORD Data2;
  WORD Data3;
  BYTE Data4[8];
} GUID;
#pragma pack(pop)

#endif  // define(WIN32)


/*
 * dpsockaddr
 * Thanks to macOS, the socakkdr_in structure has a sin_len at the beginning
 * that messes up the byte order of the sin_family value. Thus, this is an
 * os-independent use
 */
#pragma pack(push, 1)
typedef struct {
  uint16_t sin_family;
  uint16_t sin_port;
  uint32_t sin_addr;
  uint8_t sin_zero[8];
} dpsockaddr;
#pragma pack(pop)

/*
 * DPMSG_HEADER
 *
 * The DPMSG_HEADER is prepended to all DirectPlay Protocol messages and
 * contains an identifier that describes each message structure.
 */
#pragma pack(push, 1)
typedef struct {
  DWORD cbSize : 20;    //  Indicates the size of the message
  DWORD token : 12;     //  Describes high-level message
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
  dpsockaddr sockAddr;  //  16 bytes of data containing the sockets
  STR signature[4];     //  MUST be set to the value 0x79616c70
                        //  (ASCII 'play')
  WORD command;         //  For messages below
  WORD version;         //  MUST be set to the version number of
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
  DWORD dwUser4;      // Max Rank
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

/*
 * DPLAYI_PACKEDPLAYER
 *
 * This structure contains data related to players or groups
 */
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

/*
 * DPLAYI_SUPERPACKEDPLAYER
 *
 * This structure is used to transmit player or group-related data.
 */
#pragma pack(push, 1)
typedef struct {
  DWORD dwSize;            //  The size of the fixed player header =
                           //  16
  DWORD dwFlags;           //  Player flags. Player Flags MUST be 0 or
                           //  more of the following values:
                           //    SP (1 bit): The player is the system
                           //    player.
                           //    NS (1 bit): The player is the name
                           //    server (host). It MUST be combined
                           //    with SP.
                           //    PG (1 bit): The player belongs to a
                           //    group. This flag MUST be set for
                           //    system players, for other players
                           //    that have been added to a group using
                           //    DPMSG_ADDPLAYERTOGROUP, or for groups
                           //    that have been added to a group using
                           //    DPMSG_ADDSHORTCUTTOGROUP.
                           //    PL (1 bit): The player is on the
                           //    sending machine. This flag does not
                           //    have meaning on other machines and
                           //    MUST be ignored on receipt.
                           //    X (28 bits): All bits that have this
                           //    label SHOULD be set to zero when sent
                           //    and MUST be ignored on receipt.
  DWORD ID;                //  MUST contain the player ID of the
                           //  player that is described in this
                           //  structure.
  struct {  //  A bit field that indicates which
                           //  optional fields are present. The
                           //  PlayerInfoMask field MUST be a bitmask
                           //  that is composed of the following fields:
    bool shortNamePresent : 1;       //    SN (1 bit): MUST be set if the
                           //    ShortName field is present in the
                           //    structure.
    bool longNamePresent : 1;        //    LN (1 bit): MUST be set if the
                           //    LongName field is present in the
                           //    structure.
    char serviceProviderLength : 2;        //    SL (2 bits): MUST be set if the
                           //    ServiceProviderDataLength field is
                           //    present in the structure. SL MUST be
                           //    set to one of the following values.
                           //      0x01 = If the
                           //      ServiceProviderDataLength field
                           //      occupies 1 byte.
                           //      0x02 = If the
                           //      ServiceProviderDataLength field
                           //      occupies 2 bytes.
                           //      0x03 = If the
                           //      ServiceProviderDataLength field
                           //      occupies 4 bytes.
    char playerDataLength : 2;                       //    PD (2 bits): MUST be set if the
                           //    PlayerDataLength field is present in
                           //    the structure. PD MUST be set to one
                           //    of the following values:
                           //      0x01 = If the PlayerDataLength
                           //      field occupies 1 byte.
                           //      0x02 = If the PlayerDataLength
                           //      field occupies 2 bytes.
                           //      0x03 = If the PlayerDataLength
                           //      field occupies 4 bytes.
    char playerCountLength : 2;                       //    PC (2 bits): MUST be set if the
                           //    PlayerCount field is present in the
                           //    structure. PC MUST be set to one of
                           //    the following values:
                           //      0x01 = If the PlayerCount field
                           //      occupies 1 byte.
                           //      0x02 = If the PlayerCount field
                           //      occupies 2 bytes.
                           //      0x03 = If the PlayerCount field
                           //      occupies 4 bytes.
    bool parentIDPresent : 1;                       //    PI (1 bit): MUST be set if the
                           //    ParentID field is present in the
                           //    structure.
    char shortcutCountLength : 2;                       //    SC (2 bits): MUST be set if the
                           //    ShortcutCount field is present in the
                           //    structure. SC MUST be set to one of
                           //    the following values:
                           //      0x01 = If the ShortcutCount field
                           //      occupies 1 byte.
                           //      0x02 = If the ShortcutCount field
                           //      occupies 2 bytes.
                           //      0x03 = If the ShortcutCount field
                           //      occupies 4 bytes.
    int x : 21;            //    X (21 bits): All bits with this label
  } dwPlayerInfoMask;                       //    SHOULD be set to zero when sent and
                           //    MUST be ignored on receipt.
  DWORD dwSystemPlayerID;  //  If the DPLAYI_PLAYER_SYSPLAYER flag is
                           //  set in the Flags field, this field MUST
                           //  contain the protocol version for the
                           //  machine hosting the protocol. If the
                           //  DPLAYI_PLAYER_SYSPLAYER flag is not
                           //  set, this field MUST contain the ID of
                           //  the system player for this game.  When
                           //  the protocol version is used for a
                           //  system player, it will be one of the
                           //  following values:
                           //    9 = First version documented.
                           //    10 = New Hosts send
                           //    DPMSG_IAMNAMESERVER as first message
                           //    when they become the new host.
                           //    11 = No Change.
                           //    12 = The version in which DirectPlay
                           //    Voice was introduced. Does not affect
                           //    any of the core logic.
                           //    13 = Added DPMSG_CREATEPLAYERVERIFY
                           //    message.
                           //    14 = No Change.
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
  //  ShortcutIDCount multiplied by four.I
} DPLAYI_SUPERPACKEDPLAYER;
#pragma pack(pop)
enum class SUPERPACKEDPLAYERFLAGS {
  issystemplayer = 0x1,
  isnameserver = 0x2,
  isingroup = 0x4,
  islocalplayer = 0x8
};
/*
 * DPSECURITYDESC
 *
 * This structure describes the security properties of a session instance.
 */
#pragma pack(push, 1)
typedef struct {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwSSPIProvider;
  DWORD dwCAPIProvider;
  DWORD dwCAPIProviderType;
  DWORD dwEncryptionAlgorithm;
} DPSECURITYDESC;
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
  DPSESSIONDESC2 dpSessionInfo;  //  Session info
  DWORD dwNameOffset;            //  offsets, in octets from the beginning
                                 //  of the message in dpSessionInfo to the
                                 //  beginning of the message in
                                 //  szSessionName
  WSTR szSessionName[];          //  Name of the session
} DPMSG_ENUMSESSIONSREPLY;
#pragma pack(pop)

/*
 * DPMSG_ENUMSESSIONS
 *
 * This packet is sent by the client to request an enumeration of DirectPlay
 * sessions
 */
#define DPSYS_ENUMSESSIONS 0x0002
#pragma pack(push, 1)
typedef struct {
  GUID guidApplication;    //  MUST be set to the application
                           //  identifier for the session.
  DWORD dwPasswordOffset;  //  MUST be set to the offset, in octets,
                           //  of the password from the beginning of
                           //  the message.
  DWORD dwFlags;           //  AV, AL, X, PR, Y
  // WSTR szPassword[];     //  a null-terminated Unicode string that
  //  contains the password.
} DPMSG_ENUMSESSIONS;
#pragma pack(pop)
enum ENUMSESSIONSFLAGS {
  joinablesessions = 0x1,  //  Enumerate sessions that can be joined
  allsessions = 0x2,       //  Enumerate sessions even if they cannot be joined
  unksessions = 0x10,      //  Unknown but used
  passwordprotectedsessions =
      0x40  // Enumerate sessions even if they are password protected
};

/*
 * DPMSG__ENUMPLAYERSREPLY
 *
 * This packet can be sent in response to a DPMSG_ENUMPLAYER message or a
 * DPMSG_ADDFORWARDREQUEST message
 */
#define DPSYS_ENUMPLAYERSREPLY 0x0003
#pragma pack(push, 1)
typedef struct {
  DWORD dwPlayerCount;           //  number of players
  DWORD dwGroupCount;            //  number of groups
  DWORD dwPlayerOffset;          //  the offset of the PlayerInfo field
  DWORD dwShortcutCount;         //  MUST be ignored on receipt
  DWORD dwDescriptionOffset;     //  the offset of the SessionDesc field
  DWORD dwNameOffest;            //  The offset to the SessionName field
  DWORD dwPasswordOffset;        //  The offset to the Password field
  DPSESSIONDESC2 dpSessionDesc;  //  Session description information
  BYTE data[];
  //  WSTR            szSessionName[];      //  Name for the session
  //  WSTR            szPassword[];         //  Password
  //  DPLAYI_PACKEDPLAYER playerInfo[];
} DPMSG_ENUMPLAYERSREPLY;
#pragma pack(pop)

/*
 * DPMSG_DPSP_MSG_ENUMPLAYER
 *
 * This packet is sent to the server to request an enumeration of players
 */
#define DPSYS_ENUMPLAYER 0x0004
#pragma pack(push, 1)
typedef struct {
} DPMSG_ENUMPLAYER;
#pragma pack(pop)

/*
 * DPMSG_REQUESTPLAYERID
 *
 * This packet is sent to the host to request a new player ID
 * Two of these messages are often sent to optain two different IDs
 * the Player ID (dwFlags=8) and the System Player ID  (dwFlags=9)
 */
#define DPSYS_REQUESTPLAYERID 0x0005
#pragma pack(push, 1)
typedef struct {
  DWORD dwFlags;
  //  bit 0 = This player is the system player
  //  bit 1/2 = 0
  //  bit 3 = Player is on the sending machine
  //  bits 4-31 = 0
} DPMSG_REQUESTPLAYERID;
#pragma pack(pop)
enum REQUESTPLAYERIDFLAGS {
  issystemplayer = 0x1,
  isnameserver = 0x2,
  islocalplayer = 0x4,
  unknown = 0x8,
  issecuresession = 0x200
};

/*
 * DPMSG_REQUESTGROUPID
 *
 * This packet is sent to the host to request a new group identifier.
 */
#define DPSYS_REQUESTGROUPID 0x0006
#pragma pack(push, 1)
typedef struct {
  DWORD dwFlags;
} DPMSG_REQUESTGROUPID;
#pragma pack(pop)

/*
 * DPMSG_REQUESTPLAYERREPLY
 *
 * This packet is sent in response to a DPSP_MSG_REQUESTPLAYERID  or
 * DPMSG_REQUESTGROUPID  message.
 */
#define DPSYS_REQUESTPLAYERREPLY 0x0007
#pragma pack(push, 1)
typedef struct {
  DWORD dwID;
  DPSECURITYDESC dpSecurityDesc;
  DWORD dwSSPIProviderOffsert;  //  Offset of the Security Support Provider
                                //  Interface (SSPI) provider name from the
                                //  beginning of the message. Zero means
                                //  that the session is not secure.
  DWORD CAPIProviderOffset;     //  The offset of the Crypto API provider
                                //  name from the beginning of the message.
                                //  Zero means that the session will
                                //  not use encryption.
  DWORD Results;                //  MUST be set to a Win32 HRESULT error
                                //  code. If 0, the request succeeded; if
                                //  nonzero, indicates the reason the
                                //  request failed. For a complete list of
                                //  HRESULT codes.
  BYTE data[];
  // WSTR  szSSPIProvider[];        //  Null-terminated Unicode string that
  //  contains the SSPI name. If no SSPI
  //  provider is specified, the session
  //  is not a secure session.
  //  WSTR  szCAPIProvider[];       //  CAPIProvider (variable):
  //  Null-terminated Unicode string that
  //  contains the Crypto API provider name.
  //  For a list of provider names, see
  //  Cryptographic Provider Names.
} DPMSG_REQUESTPLAYERREPLY;
#pragma pack(pop)

/*
 * DPMSG_CREATEPLAYER
 *
 * This packet is sent to indicate that a new player has been created.
 */
#define DPSYS_CREATEPLAYER 0x0008
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;          //  ID Player to whom the
                         //  message is being sent.
  DWORD dwGroupID;       //  GroupID (4 bytes): Ignored
  DWORD dwCreateOffset;  //  Offset of the PlayerInfo field. = 28
  DWORD dwPasswordOffset;
  BYTE data[];
  //  DPLAYI_PACKEDPLAYER dpPlayerInfo;
  //  WORD  cbReserved1;
  //  DWORD dwReserved2;
} DPMSG_CREATEPLAYER;
#pragma pack(pop)

/*
 * DPMSG_CREATEGROUP
 *
 * This packet is sent to indicate that a new group has been created.
 */
#define DPSYS_CREATEGROUP 0x0009
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;                     //  SHOULD be set to zero
  DWORD dwPlayerID;                 //  MUST be set to the group ID of the
                                    //  newly created group.
  DWORD dwGroupID;                  //  SHOULD be set to zero when sent
  DWORD dwCreateOffset;             //  MUST be set to the offset of the
                                    //  GroupInfo field. MUST be set to 28
  DWORD dwPasswordOffset;           //  SHOULD be set to zero
  DPLAYI_PACKEDPLAYER dpGroupInfo;  //  Structure that contains information
                                    //  about the group to be created.
} DPMSG_CREATEGROUP;
#pragma pack(pop)

/*
 * DPSYS_PLAYERMESSAGE
 *
 * This packet is used to send a player-to-player message.
 */
#define DPSYS_PLAYERMESSAGE 0x000A
#pragma pack(push, 1)
typedef struct {
  union {
    DWORD dwSize;   //  Indicates the size of the message (&)
                    //  operation with the token field and
                    //  0x000FFFFF.
    DWORD dwToken;  //  Describes high-level message
                    //  characteristics. (&) operation with the
                    //  size field and 0xFFF00000. (FAB, CAB, BAB)
  };
  sockaddr_in sockaddr;  //  Not used to transmit data. This field
                         //  is a placeholder within the packet to
                         //  be used by the sender and the receiver
                         //  before the packet is sent or after it
                         //  is received
  DWORD dwIDFrom;        //  ID of the player who is the source of
                         //  the message.
  DWORD dwIDTo;          //  ID of the player who is the destination
                         //  of the message.
  BYTE data[];           //  Player messages are the primary
                         //  application method of communication
                         //  between DirectPlay applications. They
                         //  are distinguished from other DirectPlay
                         //  messages by the format of their header
                         //  and the lack of the presence of the
                         //  "play" signature in the header. The
                         //  PlayerMessage portion of the message
                         //  contains an application-specific
                         //  payload.
} DPMSG_PLAYERMESSAGE;
#pragma pack(pop)

/*
 * DPMSG_DELETEPLAYER
 *
 * This packet is sent to indicate that a player has been deleted.
 */
#define DPSYS_DELETEPLAYER 0x000B
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  To the player ID of the newly deleted
                           //  player.
  DWORD dwGroupID;         //  SHOULD be set to zero
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_DELETEPLAYER;
#pragma pack(pop)

/*
 * DPMSG_DELETEGROUP
 *
 * This packet is sent when a group is deleted
 */
#define DPSYS_DELETEGROUP 0x000C
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  SHOULD be set to zero
  DWORD dwGroupID;         //  The group ID of the newly deleted
                           //  group.
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_DELETEGROUP;
#pragma pack(pop)

/*
 * DPMSG_ADDPLAYERTOGROUP
 *
 * This packet is sent from one participant to other participants
 * when a player is added to a group.
 */
#define DPSYS_ADDPLAYERTOGROUP 0x000D
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  Identifier of the player to add to the
                           //  group specified by the GroupID field.
  DWORD dwGroupID;         //  Identifier of the group to which the
                           //  player will be added.
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_ADDPLAYERTOGROUP;
#pragma pack(pop)

/*
 * DPMSG_DELETEPLAYERFROMGROUP
 *
 * This packet is sent to indicate that a player has been deleted from a group
 */
#define DPSYS_DELETEPLAYERFROMGROUP 0x000E
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  The player ID of the newly deleted
                           //  player.
  DWORD dwGroupID;         //  The group ID that contained the deleted
                           //  player.
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_DELETEPLAYERFROMGROUP;
#pragma pack(pop)

/*
 * DPMSG_PLAYERDATACHANGED
 *
 * This packet is sent to inform all participants that the data of a player has
 * changed.
 */
#define DPSYS_PLAYERDATACHANGED 0x000F
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;        //  MUST be set to zero.
  DWORD dwPlayerID;    //  The identifier of the player whose data
                       //  is being set.
  DWORD dwDataSize;    //  The length of PlayerData
  DWORD dwDataOffset;  //  The offset of PlayerData
  BYTE playerData[];   //  Game data that contains DataSize octets
                       //  of changed data associated with the player.
} DPMSG_PLAYERDATACHANGED;
#pragma pack(pop)

/*
 * DPMSG_PLAYERNAMECHANGED
 *
 * This packet is sent to inform all participants that the name of a player has
 * changed.
 */
#define DPSYS_PLAYERNAMECHANGED 0x0010
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;         //  MUST be set to zero.
  DWORD dwPlayerID;     //  The identifier of the player whose name
                        //  is being changed.
  DWORD dwShortOffset;  //  The offset of the ShortName field
  DWORD dwLongOffset;   //  The offset of the LongName field
  BYTE data[];
  //  WSTR  szShortName[];          //  Null-terminated Unicode string that
  //  contains the new short name.
  //  WSTR  szLongName[];           //  Null-terminated Unicode string that
  //  contains the new long name.
} DPMSG_PLAYERNAMECHANGED;
#pragma pack(pop)

/*
 * DPMSG_GROUPDATACHANGED
 *
 * This packet is sent to inform all participants that group data has changed.
 */
#define DPSYS_GROUPDATACHANGED 0x0011
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;        //  SHOULD be set to zero
  DWORD dwGroupID;     //  The identifier of the groups whose data
                       //  is being set.
  DWORD dwDataSize;    //  The length of GroupData.
  DWORD dwDataOffset;  //  The offset of GroupData
  BYTE groupData[];    //  Byte array that contains application
                       //  data associated with the groups.
} DPMSG_GROUPDATACHANGED;
#pragma pack(pop)

/*
 * DPMSG_GROUPNAMECHANGED
 *
 * This packet is sent to inform all participants that a group name has
 * changed.
 */
#define DPSYS_GROUPNAMECHANGED 0x0012
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;         //  SHOULD be set to zero
  DWORD dwGroupID;      //  The identifier of the group whose data
                        //  is being set.
  DWORD dwShortOffset;  //  The offset of the ShortName field
  DWORD dwLongOffset;   //  The offset of the LongName field
  BYTE data[];
  // WSTR  szShortName[];           //  Null-terminated Unicode string that
  //  contains the new short name.
  // WSTR  szLongName[];            //  Null-terminated Unicode string that
  //  contains the new long name.
} DPMSG_GROUPNAMECHANGED;
#pragma pack(pop)

/*
 * DPMSG_ADDFORWARDREQUEST
 *
 * This packet is sent to forward a message to a downstream player.
 */
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

/*
 * DPMSG_PACKET
 *
 * This packet contains player-to-player data that is part of a larger message
 * that does not fit within the maximum transmission unit (MTU) size of the
 * transport.
 */
#define DPSYS_PACKET 0x0015
#pragma pack(push, 1)
typedef struct {
  GUID guidMessage;      //  An identifier that uniquely identifies
                         //  the message to which the packet belongs.
  DWORD dwPacketIndex;   //  The index of the packet in the series
                         //  of packets that make up the message.
  DWORD dwDataSize;      //  The total size of the data in the
                         //  packet
  DWORD dwOffset;        //  The offset of this packet in the larger
                         //  message to be transmitted.
  DWORD dwTotalPackets;  //  The total number of packets that are
                         //  used to transmit this message.
  DWORD dwMessageSize;   //  The size of the buffer that will
                         //  contain the entire message.
  DWORD dwPacketOffset;  //  The offset in the message of the actual
                         //  packet data.
  BYTE packetData[];     //  Array of DataSize bytes that contains
                         //  the packet data. PacketData is a
                         //  fragment of a large message that spans
                         //  multiple packets because it exceeded
                         //  the MTU size of the network. When all
                         //  fragments have been reassembled, the
                         //  large message must contain a complete
                         //  packet.
} DPMSG_PACKET;
#pragma pack(pop)

/*
 * DPMSG_PING
 *
 * This packet is used to keep the UDP session active and to optimize the
 * protocol.
 */
#define DPSYS_PING 0x0016
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;     //  The identifier of the player who sent
                      //  the ping.
  DWORD dwTickCount;  //  The number of milliseconds that have
                      //  elapsed since the computer system was started.
} DPMSG_PING;
#pragma pack(pop)

/*
 * DPMSG_PINGREPLY
 *
 * This packet is sent in response to a DPSP_MSG_PING message
 */
#define DPSYS_PINGREPLY 0x0017
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;     //  The identifier of the player who sent
                      //  the ping for which this is a response.
  DWORD dwTickCount;  //  The value in the DPSP_MSG_PING for
                      //  which this is the reply.
} DPMSG_PINGREPLY;
#pragma pack(pop)

/*
 * DPMSG_YOUAREDEAD
 *
 * This packet is sent in response to a DPSP_MSG_PING message when the sender
 * of the ping is not recognized as a player who belongs to the active
 * session
 */
#define DPSYS_YOUAREDEAD 0x0018
#pragma pack(push, 1)
typedef struct {
} DPMSG_YOUAREDEAD;
#pragma pack(pop)

/*
 * DPMSG_PLAYERWRAPPER
 *
 * This packet provides a wrapper message for a DPSP_MSG_PLAYERMESSAGE  packet.
 */
#define DPSYS_PLAYERWRAPPER 0x0019
#pragma pack(push, 1)
typedef struct {
  BYTE playerMessage[];
} DPMSG_PLAYERWRAPPER;
#pragma pack(pop)

/*
 * DPMSG_SESSIONDESCCHANGED
 *
 * This packet is sent to notify players that a session description
 * changed.
 */
#define DPSYS_SESSIONDESCCHANGED 0x001A
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;                  //  MUST be set to zero.
  DWORD dwSessionNameOffset;     //  The offset to the SessionName field
  DWORD dwPasswordOffset;        //  The offset to the Password field
  DPSESSIONDESC2 dpSessionDesc;  //  structure containing the session
                                 //  description.
  BYTE data[];
  // WSTR  szSessionName[];         //  Null-terminated Unicode string
  //  containing the session name.
  // WSTR  szPassword[];            //  Null-terminated Unicode string
  //  containing the session password.
} DPMSG_SESSIONDESCCHANGED;
#pragma pack(pop)

/*
 * DPMSG_CHALLENGE
 *
 * This packet is used to request a security token.
 */
#define DPSYS_CHALLENGE 0x001C
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;        //  The system player ID on the sender's
                         //  computing system.
  DWORD dwDataSize;      //  The size of the SecurityToken field.
  DWORD dwDataOffset;    //  The offset from the beginning of the
                         //  message to the SecurityToken field.
  BYTE securityToken[];  //  Opaque security token whose size is
                         //  specified by the DataSize field.
} DPMSG_CHALLENGE;
#pragma pack(pop)

/*
 * DPMSG_ACCESSGRANTED
 *
 * This packet is sent to a DirectPlay client after the client has successfully
 * been authenticated as a member of the session.
 */
#define DPSYS_ACCESSGRANTED 0x001D
#pragma pack(push, 1)
typedef struct {
  DWORD dwPublicKeySize;    //  The size of the PublicKey field
  DWORD dwPublicKeyOffset;  //  The offset from the beginning of the
                            //  packet to the PublicKey field.
  BYTE publicKey[];         //  Array of bytes that contains the
                            //  sender's signed public key.
} DPMSG_ACCESSGRANTED;
#pragma pack(pop)

/*
 * DPMSG_LOGONDENIED
 *
 * This packet is sent to indicate that a logon failed
 */
#define DPSYS_LOGONDENIED 0x001E
#pragma pack(push, 1)
typedef struct {
} DPMSG_LOGONDENIED;
#pragma pack(pop)

/*
 * DPMSG_AUTHERROR
 *
 * This packet is sent to indicate the reason that authentication failed.
 */
#define DPSYS_AUTHERROR 0x001F
#pragma pack(push, 1)
typedef struct {
  DWORD dwError;  //  The reason authentication failed. The
                  //  values associated with each error can
                  //  be found in [MS-ERREF].
} DPMSG_AUTHERROR;
#pragma pack(pop)

/*
 * DPMSG_NEGOTIATE
 *
 * This packet is sent to indicate to the server that the client is seeking to
 * initiate a secure connection.
 */
#define DPSYS_NEGOTIATE 0x0020
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;        //  The system player ID on the sender's
                         //  computing system.
  DWORD dwDataSize;      //  The size, in octets, of the SecurityToken field.
  DWORD dwDataOffset;    //  The offset from the beginning of the
                         //  message to the SecurityToken field.
  BYTE securityToken[];  //  Opaque security token whose size is
                         //  specified by the DataSize field.
} DPMSG_NEGOTIATE;
#pragma pack(pop)

/*
 * DPMSG_CHALLENGERESPONSE
 *
 * This packet is sent in response to a DPSP_MSG_CHALLENGE message.
 */
#define DPSYS_CHALLENGERESPONSE 0x0021
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;        //  The system player ID for the sender's
                         //  computing system.
  DWORD dwDataSize;      //  The size of the message in the
                         //  SecurityToken field.
  DWORD dwDataOffset;    //  The offset, in octets, from the
                         //  beginning of the message to the
                         //  SecurityToken field.
  BYTE securityToken[];  //  Opaque security token whose size is
                         //  specified by the DataSize field.
} DPMSG_CHALLENGERESPONSE;
#pragma pack(pop)

/*
 * DPMSG_SIGNED
 *
 * This packet is used to send a signed message along with its signature.
 */
#define DPSYS_SIGNED 0x0022
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;         //  The system player ID of the sender.
  DWORD dwDataOffset;     //  The offset of the DirectPlay message.
  DWORD dwDataSize;       //  The size of the Message field
  DWORD dwSignatureSize;  //  he size of the signature
  DWORD dwFlags;          //  MUST be set to one or more of the following:
                          //    SS (1 bit): If set, the message was
                          //    signed by the Security Support
                          //    Provider Interface (SSPI) specified
                          //    when the game session was
                          //    established.
                          //    SC (1 bit): If set, the message was
                          //    signed by the cryptographic algorithm
                          //    specified when the game session was
                          //    established.
                          //    EC (1 bit): Message was encrypted by
                          //    Crypto API.
                          //    X (29 bits): All bits with this label
                          //    SHOULD be set to zero when sent and
                          //    MUST be ignored on receipt.
  BYTE data[];
  // BYTE  message[];               //  Array of bytes that contains the
  //  DirectPlay message. The Message field
  //  can contain any DirectPlay 4 Protocol
  //  message. However, the message MUST
  //  begin with the Signature field of the
  //  DPMSG_HEADER rather than the entire
  //  DPMSG_HEADER structure. Once
  //  authentication is negotiated,
  //  DirectPlay sends all messages in as
  //  signed, except the following:
  //    The DPSP_MSG_ADDFORWARDREQUEST
  //    and DPSP_MSG_SESSIONDESCCHANGED
  //    messages are sent signed and
  //    encrypted.
  //  The higher layer determines whether the
  //  DPSP_MSG_PLAYERMESSAGE or
  //  DPSP_MSG_ASK4MULTICASTGUARANTEED
  //  message SHOULD be sent signed and/or
  //  encrypted.  The DPSP_MSG_PING and
  //  DPSP_MSG_PINGREPLY messages are not
  //  signed or encrypted.
  // BYTE  signature[];             // Array of bytes that contains the message
  //  signature.
} DPMSG_SIGNED;
#pragma pack(pop)

/*
 * DPMSG_ADDFORWARDREPLY
 *
 * This packet is sent in response to a DPMSG_ADDFORWARDREQUEST message when
 * there is an error.
 */
#define DPSYS_ADDFORWARDREPLY 0x0024
#pragma pack(push, 1)
typedef struct {
  DWORD dwError;  //  Indicates the reason that the
                  //  DPMSG_ADDFORWARD message failed. For a
                  //  complete list of DirectPlay 4 HRESULT
                  //  codes, see [MS-ERREF].
} DPMSG_ADDFORWARDREPLY;
#pragma pack(pop)

/*
 * DPMSG_ASK4MULTICAST
 *
 * This packet is sent to request that the server forward a message to players
 * in a specified group.
 */
#define DPSYS_ASK4MULTICAST 0x0025
#pragma pack(push, 1)
typedef struct {
  DWORD dwGroupTo;          //  Identifier of the group that is the
                            //  target of the request.
  DWORD dwPlayerFrom;       //  Identifier of the player that
                            //  originated the request.
  DWORD dwMessageOffset;    //  Offset to the MulticastMessage field.
  BYTE MulticastMessage[];  //  An array of octets that contains the
                            //  message to forward. This field MUST
                            //  contain a complete DirectPlay 4
                            //  Protocol message. However, the message
                            //  MUST begin with the Signature field of
                            //  the DPMSG_HEADER rather than the entire
                            //  DPMSG_HEADER structure.
} DPMSG_ASK4MULTICAST;
#pragma pack(pop)

/*
 * DPMSG_ASK4MULTICASTGUARANTEED
 *
 * This packet is used to request that the server forward a message to players
 * in a specified group using the guaranteed messaging mechanism.
 */
#define DPSYS_ASK4MULTICASTGUARANTEED 0x0026
#pragma pack(push, 1)
typedef struct {
  DWORD dwGroupTo;          //  Identifier of the group that is the
                            //  target of the request.
  DWORD dwPlayerFrom;       //  Identifier of the player that
                            //  originated the request.
  DWORD dwMessageOffset;    //  Offset to the MulticastMessage field.
  BYTE MulticastMessage[];  //  An array of octets that contains the
                            //  message to forward. This field MUST
                            //  contain a complete DirectPlay 4
                            //  Protocol message. However, the message
                            //  MUST begin with the Signature field of
                            //  the DPMSG_HEADER rather than the entire
                            //  DPMSG_HEADER structure.
} DPMSG_ASK4MULTICASTGUARANTEED;
#pragma pack(pop)

/*
 * DPMSG_ADDSHORTCUTTOGROUP
 *
 * Desc
 */
#define DPSYS_ADDSHORTCUTTOGROUP 0x0027
#pragma pack(push, 1)
typedef struct {
} DPMSG_ADDSHORTCUTTOGROUP;
#pragma pack(pop)

/*
 * DPMSG_DELETEGROUPFROMGROUP
 *
 * This packet is sent to add a shortcut to a group.
 */
#define DPSYS_DELETEGROUPFROMGROUP 0x0028
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  Identifier of the player to whom the
                           //  message is being sent.
  DWORD dwChildGroupID;    //  Identifier of the group to add to the
                           //  group specified by ParentGroupID.
  DWORD dwParentGroupID;   //  The containing group identifier.
  DWORD dwCreateOffset;    //  SHOULD be set to zero
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
} DPMSG_DELETEGROUPFROMGROUP;
#pragma pack(pop)

/*
 * DPMSG_SUPERENUMPLAYERSREPLY
 *
 * This packet can be sent in response to a DPMSG_ENUMPLAYER message or a
 * DPMSG_ADDFORWARDREQUEST message.
 */
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

/*
 * DPMSG_KEYEXCHANGE
 *
 * This packet is used to send the client's public key to the server
 */
#define DPSYS_KEYEXCHANGE 0x002B
#pragma pack(push, 1)
typedef struct {
  DWORD dwSessionKeySize;    //  The size of the SessionKey field.
  DWORD dwSessionKeyOffest;  //  The offset to the SessionKey field.
  DWORD dwPublicKeySize;     //  The size of the PublicKey field.
  DWORD dwPublicKeyOffset;   //  The offset from the beginning of the
                             //  message to the PublicKey field.
  BYTE data[];
  // BYTE  sessionKey[];            //  Array of bytes that contains the key
  //  used to encrypt data.
  // BYTE  publicKey[];             //  Array of bytes that contains the
  //  client's public key.
} DPMSG_KEYEXCHANGE;
#pragma pack(pop)

/*
 * DPMSG_KEYEXCHANGEREPLY
 *
 * This packet is sent in response to a DPMSG_KEYEXCHANGE message that contains
 * the server's public key.
 */
#define DPSYS_KEYEXCHANGEREPLY 0x002C
#pragma pack(push, 1)
typedef struct {
  DWORD dwSessionKeySize;    //  The size of the SessionKey field.
  DWORD dwSessionKeyOffset;  //  The offset to the SessionKey field.
  DWORD dwPublicKeySize;     //  SHOULD be set to zero
  DWORD dwPublicKeyOffset;   //  SHOULD be set to zero
  BYTE sessionKey[];         //  Array of bytes that contains the key
                             //  used to encrypt data.
} DPMSG_KEYEXCHANGEREPLY;
#pragma pack(pop)

/*
 * DPMSG_CHAT
 *
 * This packet is used to exchange text between players.
 */
#define DPSYS_CHAT 0x002D
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;         //  The identifier of the sending player.
  DWORD dwIDTo;           //  The identifier of the destination
                          //  player or group.
  DWORD dwFlags;          //  Chat flags. MUST be set to one of the
                          //  following values:
                          //    GS (1 bit): If this bit is set, send
                          //    the message using a guaranteed send
                          //    method. If this bit is clear, send
                          //    the message using a nonguaranteed
                          //    send method. Note Determining whether
                          //    to send the message guaranteed can be
                          //    inferred from the DPSP_MSG_HEADER and
                          //    the transport method. Use of the GS
                          //    flag is not required.
                          //    X (31 bits): Not used. SHOULD be set
                          //    to zero when sent and MUST be ignored
                          //    on receipt.
  DWORD dwMessageOffset;  //  The offset to the ChatMessage field
  WSTR chatMessage[];     //  Null-terminated Unicode string that
                          //  contains the contents of the chat message.
} DPMSG_CHAT;
#pragma pack(pop)

/*
 * DPMSG_ADDFORWARD
 *
 * This packet is sent to inform a game instance of the existence of other game
 * instances.
 */
#define DPSYS_ADDFORWARD 0x002E
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;                //  Identifier of the player to whom the
                               //  message is being sent.
  DWORD dwPlayerID;            //  Identifier of the affected player.
  DWORD dwGroupID;             //  Identifier of the affected group.
  DWORD dwCreateOffset;        //  Offset of the PlayerInfo field. It MUST
                               //  be set to 28 (0x0000001C).
  DWORD dwPasswordOffset;      //  Not used. It MUST be ignored on
                               //  receipt.
  DPLAYI_PACKEDPLAYER player;  //  MUST be set to a DPLAYI_PACKEDPLAYER
                               //  structure that contains information
                               //  about the system player on the newly
                               //  added machine.
} DPMSG_ADDFORWARD;
#pragma pack(pop)

/*
 * DPMSG_ADDFORWARDACK
 *
 * This packet is sent in response to a DPSP_MSG_ADDFORWARD message.
 */
#define DPSYS_ADDFORWARDACK 0x002F
#pragma pack(push, 1)
typedef struct {
  DWORD dwID;  //  Identifier of the player for whom a
               //  DPMSG_ADDFORWARD message was sent
} DPMSG_ADDFORWARDACK;
#pragma pack(pop)

/*
 * DPMSG_PACKET2_DATA
 *
 * The packet contains player-to-player data that is part of a larger message
 * that does not fit within the maximum transmission unit (MTU) size of the
 * transport. It MUST be acknowledged with a DPMSG_PACKET2_ACK message
 */
#define DPSYS_PACKET2_DATA 0x0030
#pragma pack(push, 1)
typedef struct {
  GUID guidMessage;      //  Identifier that uniquely identifies the
                         //  message to which the packet belongs.
  DWORD dwPacketIndex;   //  The index of the packet in the series
                         //  of packets that make up the message.
  DWORD dwDataSize;      //  The total size of the data in the packet.
  DWORD dwOffset;        //  The offset of this packet in the larger
                         //  message to be transmitted.
  DWORD dwTotalPackets;  //  The total number of packets that are
                         //  used to transmit this message.
  DWORD dwMessageSize;   //  The size of the buffer that will
                         //  contain the entire message.
  DWORD dwPacketOffset;  //  The offset into the message of the
                         //  actual packet data.
  BYTE packetData[];     //  Array of DataSize bytes that contains
                         //  the packet data.
} DPMSG_PACKET2_DATA;
#pragma pack(pop)

/*
 * DPMSG_PACKET2_ACK
 *
 * This packet is sent in response to a DPMSG_PACKET2_DATA message.
 */
#define DPSYS_PACKET2_ACK 0x0031
#pragma pack(push, 1)
typedef struct {
  GUID guidMessage;  //  Identifier of the message to which this
                     //  packet belongs.
  DWORD dwPacketID;  //  Acknowledgment (ACK) packet identifier.
} DPMSG_PACKET2_ACK;
#pragma pack(pop)

/*
 * DPMSG_IAMNAMESERVER
 *
 * This packet is sent to inform participants of the identity of the name
 * server (host).
 */
#define DPSYS_IAMNAMESERVER 0x0035
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;              //  The identifier of the destination
                             //  player.
  DWORD dwIDHost;            //  The system player ID of the new
                             //  DirectPlay host.
  DWORD dwFlags;             //  The player flags that describe the
                             //  system player of the new host.
                             //    SP (1 bit): The player is the system
                             //    player.
                             //    NS (1 bit): The player is the name
                             //    server (host). MUST be combined with
                             //    SP.
                             //    PG (1 bit): The player belongs to a
                             //    group. This flag MUST be set for
                             //    system players, for other players
                             //    that have been added to a group using
                             //    DPMSG_ADDPLAYERTOGROUP, or for groups
                             //    that have been added to a group using
                             //    DPMSG_ADDSHORTCUTTOGROUP.
                             //    X (29 bits): All bits that have this
                             //    label SHOULD be set to zero when sent
                             //    and MUST be ignored on receipt.
  DWORD dwSPDataSize;        //  The length of the SPData field.
  struct {                   //  If SPDataSize is nonzero, MUST be set
                             //  to the data that is used by the
                             //  DirectPlay Service Provider.  If
                             //  provided, the Windows Winsock
                             //  DirectPlay Service Provider stores the
                             //  following data in the SPData field.
    sockaddr_in streamSock;  //  A SOCKADDR_IN structure that contains
                             //  the addressing information to be used
                             //  when contacting this player over TCP.
                             //  The Address field of this SOCKADDR_IN
                             //  must be set to 0.0.0.0.
    sockaddr_in dataSock;    //  A SOCKADDR_IN structure that contains
                             //  the addressing information to be used
                             //  when contacting this player over UDP.
                             //  The Address field of this SOCKADDR_IN
                             //  must be set to 0.0.0.0.
  };
} DPMSG_IAMNAMESERVER;
#pragma pack(pop)

/*
 * DPMSG_VOICE
 *
 * This packet is used to send voice message data.
 */
#define DPSYS_VOICE 0x0036
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDFrom;    //  The player ID of the source for the
                     //  voice data.
  DWORD dwIDTo;      //  The player ID of the destination for
                     //  the voice data.
  BYTE voiceData[];  //  Variable-sized voice data payload to be
                     //  delivered to the voice layer. See [MC-DPLVP].
} DPMSG_VOICE;
#pragma pack(pop)

/*
 * DPMSG_MULTICASTDELIVERY
 *
 * This packet is used to perform a message broadcast.
 */
#define DPSYS_MULTICASTDELIVERY 0x0037
#pragma pack(push, 1)
typedef struct {
  DWORD dwGroupIDTo;        //  The identifier of the group that is the
                            //  target of the request.
  DWORD dwPlayerIDFrom;     //  The identifier of the player that is
                            //  originating the request.
  DWORD dwMessageOffset;    //  The offset from the beginning of the
                            //  message to the BroadcastMessage field.
  BYTE broadcastMessage[];  //  An array of octets that contains the
                            //  message to broadcast. This field MUST
                            //  contain a complete DirectPlay 4
                            //  Protocol message. However, the message
                            //  MUST begin with the Signature field of
                            //  the DPMSG_HEADER rather than the entire
                            //  DPMSG_HEADER structure.
} DPMSG_MULTICASTDELIVERY;
#pragma pack(pop)

/*
 * DPMSG_CREATEPLAYERVERIFY
 *
 * This message is sent as verification that a player was previously created.
 * When all of the following conditions are met, one or more
 * DPMSG_CREATEPLAYERVERIFY messages are sent in response to a
 * DPMSG_CREATEPLAYER message
 */
#define DPSYS_CREATEPLAYERVERIFY 0x0038
#pragma pack(push, 1)
typedef struct {
  DWORD dwIDTo;            //  SHOULD be set to zero
  DWORD dwPlayerID;        //  The identifier of the previously
                           //  created player.
  DWORD dwGroupID;         //  SHOULD be set to zero
  DWORD dwCreateOffset;    //  Offset of the PlayerInfo field. MUST be
                           //  set to 28 (0x0000001C).
  DWORD dwPasswordOffset;  //  SHOULD be set to zero
  BYTE data[];             //
  // DPLAYI_PACKEDPLAYER player;    //  MUST contain a DPLAYI_PACKEDPLAYER
  //  structure containing the information
  //  about the previously created player.
  // WORD  cbReserved1;             //  SHOULD be set to zero
  // WORD  dwReserved2;             //  SHOULD be set to zero
} DPMSG_CREATEPLAYERVERIFY;
#pragma pack(pop)

#endif  // INCLUDE_DPPL_DPLAY_H_
