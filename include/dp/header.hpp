#ifndef INCLUDE_DP_HEADER_HPP_
#define INCLUDE_DP_HEADER_HPP_

#include <string>
#include <memory>
#include <vector>

#include "dp/sockaddr.hpp"
#include "dp/types.h"
#include "experimental/net"

// DPMSG_HEADER
//
// The DPMSG_HEADER is prepended to all DirectPlay Protocol messages and
// contains an identifier that describes each message structure.
#pragma pack(push, 1)
typedef struct {
  DWORD cbSize : 20;      //  Indicates the size of the message
  DWORD token : 12;       //  Describes high-level message
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
  dp::sockaddr sockAddr;  //  16 bytes of data containing the sockets
  STR signature[4];       //  MUST be set to the value 0x79616c70
                          //  (ASCII 'play')
  WORD command;           //  For messages below
  WORD version;           //  MUST be set to the version number of
                          //  the protocol
} DPMSG_HEADER;
#pragma pack(pop)

namespace dp {
class header {
 public:
  enum class Token : DWORD { REMOTE = 0xFAB, FORWARD = 0xCAB, SERVER = 0xBAB };

  explicit header(BYTE *data);

  std::vector<BYTE> to_vector(void);

  DWORD size;
  Token token;
  std::experimental::net::ip::tcp::endpoint sock_addr;
  std::string signature;
  WORD command;
  WORD version;

 private:
  DWORD load_size(void);
  Token load_token(void);
  std::experimental::net::ip::tcp::endpoint load_sock_addr(void);
  std::string load_signature(void);

  DPMSG_HEADER *header_;
};
}  // namespace dp

#endif  // INCLUDE_DP_HEADER_HPP_
