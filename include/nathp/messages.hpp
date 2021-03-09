#ifndef INCLUDE_NATHP_MESSAGES_HPP_
#define INCLUDE_NATHP_MESSAGES_HPP_

#include <cstdint>

namespace nathp {
enum MSG : uint8_t { HEADER };

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
}  // namespace nathp

#endif  // INCLUDE_NATHP_MESSAGES_HPP_
