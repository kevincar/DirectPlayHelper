
#ifndef INCLUDE_DPPL_DPLAY_H_
#define INCLUDE_DPPL_DPLAY_H_

#include "dp/types.h"

/*****************************************************************************
 *
 * DirectPlay System Messages
 *
 ****************************************************************************/

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
  DWORD dwSize : 20;     //  Indicates the size of the message (&)
                         //  operation with the token field and
                         //  0x000FFFFF.
  DWORD dwToken : 12;    //  Describes high-level message
                         //  characteristics. (&) operation with the
                         //  size field and 0xFFF00000. (FAB, CAB, BAB)
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
