#ifndef INCLUDE_MESSAGE_HPP_
#define INCLUDE_MESSAGE_HPP_

#include <cstdint>
#include <vector>

#include "dppl/message.hpp"

namespace dph {
#pragma pack(push, 1)
typedef struct {
  uint32_t from_id;
  uint32_t to_id;
  uint8_t msg_command;
  uint32_t data_size;
  char data[];
} MESSAGE;
#pragma pack(pop)
enum Command { REQUESTID, REQUESTIDREPLY, FORWARDMESSAGE };

#pragma pack(push, 1)
typedef struct {
  uint32_t n_clients;
  uint32_t client_ids[];
} MSG_ENUMCLIENTS;
#pragma pack(pop)

class Message {
 public:
  Message(void);
  Message(uint32_t from, uint32_t to, Command command,
          std::vector<BYTE> payload);
  explicit Message(std::vector<uint8_t> const& data);
  Message(uint32_t from, uint32_t to, Command command, uint32_t data_size,
          char const* data);
  std::size_t size(void) const;
  std::vector<uint8_t> to_vector(void) const;

  uint32_t from_id;
  uint32_t to_id;
  Command command;
  std::vector<BYTE> payload;
};
}  // namespace dph

#endif  // INCLUDE_MESSAGE_HPP_
