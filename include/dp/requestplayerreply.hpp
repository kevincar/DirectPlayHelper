#ifndef INCLUDE_DP_REQUESTPLAYERREPLY_HPP_
#define INCLUDE_DP_REQUESTPLAYERREPLY_HPP_

#include <string>
#include <vector>

#include "dp/securitydesc.hpp"
#include "dp/types.h"

// DPMSG_REQUESTPLAYERREPLY
//
// This packet is sent in response to a DPSP_MSG_REQUESTPLAYERID or
// DPMSG_REQUESTGROUPID  message.
#define DPSYS_REQUESTPLAYERREPLY 0x0007
#pragma pack(push, 1)
typedef struct {
  DWORD dwID;
  DPSECURITYDESC dpSecurityDesc;
  DWORD dwSSPIProviderOffset;  //  Offset of the Security Support Provider
                               //  Interface (SSPI) provider name from the
                               //  beginning of the message. Zero means
                               //  that the session is not secure.
  DWORD dwCAPIProviderOffset;  //  The offset of the Crypto API provider
                               //  name from the beginning of the message.
                               //  Zero means that the session will
                               //  not use encryption.
  DWORD Results;               //  MUST be set to a Win32 HRESULT error
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

namespace dp {
class requestplayerreply {
 public:
  explicit requestplayerreply(BYTE* data);
  std::vector<BYTE> to_vector(void);

  DWORD id;
  DPSECURITYDESC security_desc;
  DWORD result;
  std::string sspi_provider;
  std::string capi_provider;

 private:
  // Pointers
  BYTE* get_sspi_provider_ptr(void);
  BYTE* get_capi_provider_ptr(void);

  // Loaders
  void load_sspi_provider(void);
  void load_capi_provider(void);

  // Assigners
  void assign_sspi_provider(void);
  void assign_capi_provider(void);

  DPMSG_REQUESTPLAYERREPLY* message_;
};
}  // namespace dp

#endif  // INCLUDE_DP_REQUESTPLAYERREPLY_HPP_
