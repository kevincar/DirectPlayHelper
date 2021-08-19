#ifndef INCLUDE_DP_HEADER_HPP_
#define INCLUDE_DP_HEADER_HPP_

#include <string>
#include <vector>

#include "dp/types.h"
#include "dp/sockaddr.hpp"
#include "experimental/net"

// DPMSG_HEADER
//
// The DPMSG_HEADER is prepended to all DirectPlay Protocol messages and
// contains an identifier that describes each message structure.
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
  dp::sockaddr sockAddr;  //  16 bytes of data containing the sockets
  STR signature[4];     //  MUST be set to the value 0x79616c70
                        //  (ASCII 'play')
  WORD command;         //  For messages below
  WORD version;         //  MUST be set to the version number of
                        //  the protocol
} DPMSG_HEADER;
#pragma pack(pop)

namespace dp {
class header {
 public:
  enum class token : DWORD { REMOTE = 0xFAB, FORWARD = 0xCAB, SERVER = 0xBAB };

  explicit header(std::shared_ptr<std::vector<BYTE>> message_data);

  DWORD get_cb_size(void);
  void set_cb_size(std::size_t size);

  token get_token(void);
  void set_token(token new_token);

  std::experimental::net::ip::tcp::endpoint get_sock_addr(void);
  void set_sock_addr(std::experimental::net::ip::tcp::endpoint const &endpoint);

  std::string get_signature(void);
  void set_signature(std::string const& signature);

  WORD get_command(void);
  void set_command(WORD command);

  WORD get_version(void);
  void set_version(WORD version);

 private:
  std::shared_ptr<std::vector<BYTE>> message_data_;
  BYTE *data_;
  DPMSG_HEADER *header_;
};
}  // namespace dp

#endif  // INCLUDE_DP_HEADER_HPP_
