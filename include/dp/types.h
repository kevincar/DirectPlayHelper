#ifndef INCLUDE_DP_TYPES_H_
#define INCLUDE_DP_TYPES_H_

#include <stdint.h>

#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#include <winsock2.h>
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

inline bool operator==(GUID const& lhs, GUID const& rhs) {
  bool data4 = true;
  for (int i = 0; i < 8; i++) {
    data4 = data4 && (lhs.Data4[i] == rhs.Data4[i]);
  }
  return lhs.Data1 == rhs.Data1 && lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 && data4;
}

#endif  // define(WIN32)

inline std::size_t get_u16string_size(std::string const& s,
                                      std::size_t dflt = 0) {
  // Accounts for the null byte pair
  return s.size() ? s.size() * 2 + 2 : dflt;
}
#endif  // INCLUDE_DP_TYPES_H_
