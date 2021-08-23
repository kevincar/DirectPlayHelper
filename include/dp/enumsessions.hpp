#ifndef INCLUDE_DP_ENUMSESSIONS_HPP_
#define INCLUDE_DP_ENUMSESSIONS_HPP_

#include <vector>
#include <string>

#include "dp/types.h"

// DPMSG_ENUMSESSIONS
//
// This packet is sent by the client to request an enumeration of DirectPlay
// sessions
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

namespace dp {
class enumsessions {
 public:
  enum class Flags : int {
    joinablesessions = 0x1,  //  Enumerate sessions that can be joined
    allsessions = 0x2,   //  Enumerate sessions even if they cannot be joined
    unksessions = 0x10,  //  Unknown but used
    passwordprotectedsessions =
        0x40  // Enumerate sessions even if they are password protected
  };

  explicit enumsessions(BYTE* data);
  std::vector<BYTE> to_vector(void);

  GUID application;
  Flags flags;
  std::string password;

 private:
  // Pointers
  BYTE* get_password_ptr(void);

  // Loaders
  void load_password(void);

  // Assigners
  void assign_password(void);

  DPMSG_ENUMSESSIONS* message_;
};
}  // namespace dp
enum ENUMSESSIONSFLAGS {};

#endif  // INCLUDE_DP_ENUMSESSIONS_HPP_
