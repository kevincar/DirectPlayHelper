#ifndef INCLUDE_DPHMESSAGE_HPP_
#define INCLUDE_DPHMESSAGE_HPP_

#include <cstdint>
#include <vector>

#pragma pack(push, 1)
typedef struct {
  uint32_t from_id;
  uint32_t to_id;
  uint8_t msg_command;
  uint32_t data_size;
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

namespace dph {
class DPHMessage {
 public:
  DPHMessage(void);
  explicit DPHMessage(std::vector<char> const& data);

  DPH_MESSAGE* get_message(void);

  std::vector<char> get_payload(void);
  void set_payload(std::vector<char> const& payload);

 private:
  std::vector<char> data_;
};
}  // namespace dph

#endif  // INCLUDE_DPHMESSAGE_HPP_
