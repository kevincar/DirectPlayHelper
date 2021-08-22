#ifndef INCLUDE_DP_SECURITYDESC_HPP_
#define INCLUDE_DP_SECURITYDESC_HPP_

#include "dp/types.h"

// DPSECURITYDESC
//
// This structure describes the security properties of a session instance.
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

#endif  // INCLUDE_DP_SECURITYDESC_HPP_
