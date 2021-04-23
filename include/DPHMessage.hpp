#ifndef INCLUDE_DPHMESSAGE_HPP_
#define INCLUDE_DPHMESSAGE_HPP_

#include <cstdint>
#include <vector>

#pragma pack(push, 1)
typedef struct {
  uint32_t from_id;
  uint32_t to_id;
  uint8_t msg_command;
  char data[];
} DPH_MESSAGE;
#pragma pack(pop)
enum DPHCommand {
  REQUESTID,
  REQUESTIDREPLY,
  ENUMCLIENTS,
  ENUMCLIENTSREPLY,
  FORWARDMESSAGE
};

#endif  // INCLUDE_DPHMESSAGE_HPP_
