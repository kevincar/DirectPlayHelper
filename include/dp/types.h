#ifndef INCLUDE_DP_TYPES_H_
#define INCLUDE_DP_TYPES_H_

#include <stdint.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <winsock2.h>
#include <windows.h>
typedef unsigned char STR;
typedef char16_t WSTR;
typedef uint64_t QWORD;
#else
#include <netinet/in.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef unsigned char STR, *LPSTR;
typedef char16_t WSTR, *LPWSTR;

// GUID
//
// Global Unique Identifier
#pragma pack(push, 1)
typedef struct {
  DWORD Data1;
  WORD Data2;
  WORD Data3;
  BYTE Data4[8];
} GUID;
#pragma pack(pop)

#endif  // define(WIN32)

// dpsockadder
//
// Thanks to macOS, the socakkdr_in structure has a sin_len at the beginning
// that messes up the byte order of the sin_family value. Thus, this is an
// os-independent use
#pragma pack(push, 1)
typedef struct {
  uint16_t sin_family;
  uint16_t sin_port;
  uint32_t sin_addr;
  uint8_t sin_zero[8];
} dpsockaddr;
#pragma pack(pop)

#endif  // INCLUDE_DP_TYPES_H_
