#ifndef INCLUDE_DP_ENUMSESSIONSREPLY_HPP_
#define INCLUDE_DP_ENUMSESSIONSREPLY_HPP_

#include <string>
#include <vector>

#include "dp/sessiondesc.hpp"
#include "dp/types.h"

// DPMSG_ENUMSESSIONSREPLY
//
// This packet is sent by the server in response to a DPSP_MSG_ENUMSESSIONS
// request
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

namespace dp {
class enumsessionsreply {
 public:
  explicit enumsessionsreply(BYTE* data);
  std::vector<BYTE> to_vector(void);

  sessiondesc session_desc;
  std::string session_name;

 private:
  BYTE* get_session_name_ptr(void);

  void load_session_name(void);

  void assign_session_name(void);
  DPMSG_ENUMSESSIONSREPLY* message_;
};
}  // namespace dp
#endif  // INCLUDE_DP_ENUMSESSIONSREPLY_HPP_
