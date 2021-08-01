#ifndef INCLUDE_MESSAGE_HPP_
#define INCLUDE_MESSAGE_HPP_

#include <cstdint>
#include <vector>

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
enum Command {
  REQUESTID,
  REQUESTIDREPLY,
  FORWARDMESSAGE
};

#pragma pack(push, 1)
typedef struct {
  uint32_t n_clients;
  uint32_t client_ids[];
} MSG_ENUMCLIENTS;
#pragma pack(pop)

class Message {
 public:
  Message(void);
  explicit Message(std::vector<char> const& data);
  Message(uint32_t from, uint32_t to, Command command, uint32_t data_size,
             char const* data);

  MESSAGE* get_message(void);

  std::vector<char> get_payload(void);
  void set_payload(std::vector<char> const& payload);

  std::vector<char> to_vector(void) const;

 private:
  std::vector<char> data_;
};
}  // namespace dph

#endif  // INCLUDE_MESSAGE_HPP_
