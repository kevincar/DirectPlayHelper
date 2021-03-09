#ifndef INCLUDE_NATHP_MESSAGES_HPP_
#define INCLUDE_NATHP_MESSAGES_HPP_

#include <cstdint>

namespace nathp {
enum MSG : uint8_t {
  HEADER,
  REGISTER,
  REGISTERREPLY,
  ENUMPEERS,
  ENUMPEERSREPLY,
  HOLEPUNCHREQUEST
};

/*
 * MSG_HEADER
 * All data sent over NATHP protocol will have this header at the beginning of
 * the data
 */
#pragma pack(push, 1)
typedef struct _MSG_HEADER {
  uint16_t size;
  MSG command;
} MSG_HEADER;
#pragma pack(pop)

/*
 * MSG_REGISTER
 * Sent by the client to the server in order to obtain both the public facing
 * endpoint and the session ID
 */
#pragma pack(push, 1)
typedef struct _MSG_REGISTER {
  uint8_t private_endpoint_size;
  char private_enpoint_str[];
} MSG_REGISTER;
#pragma pack(pop)

/*
 * MSG_REGISTERREPLY
 * Sent by the server to the client contining their session ID and their public
 * facing endpoint
 */
#pragma pack(push, 1)
typedef struct _MSG_REGISTEREPLY {
  int ID;
  uint8_t public_enpoint_size;
  char public_endpoint_str[];
} MSG_REGISTERREPLY;
#pragma pack(pop)

/*
 * MSG_ENUMPEERS
 * Sent by the client to the server to request for an enumerated list of peer
 * IDs
 */
#pragma pack(push, 1)
typedef struct _MSG_ENUMPEERS {
} MSG_ENUMPEERS;
#pragma pack(pop)

/*
 * MSG_ENUMPEERSREPLY
 * Sent by the server back to the client with a list of peers
 */
#pragma pack(push, 1)
typedef struct _MSG_ENUMPEERSREPLY {
  uint32_t nIDs;
  int IDs[];
} MSG_ENUMPEERSREPLY;
#pragma pack(pop)

/*
 * MSG_HOLEPUNCHREQUEST
 * Send by a client to the server to indicate a request to setup direct
 * peer-to-peer communcation with another connected PEER
 */
#pragma pack(push, 1)
typedef struct _MSG_HOLEPUNCHREQUEST {
  int fromID;
  int toID;
} MSG_HOLEPUNCHREQUEST;
#pragma pack(pop)
}  // namespace nathp

#endif  // INCLUDE_NATHP_MESSAGES_HPP_
