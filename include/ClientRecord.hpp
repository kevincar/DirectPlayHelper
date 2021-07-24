#ifndef INCLUDE_CLIENTRECORD_HPP_
#define INCLUDE_CLIENTRECORD_HPP_
#include <experimental/net>
#include <cstdint>
#include <vector>

namespace dph {
#pragma pack(push, 1)
typedef struct {
  uint32_t id;
  uint32_t public_address;
  uint16_t public_port;
} CLIENT_RECORD;
#pragma pack(pop)
class ClientRecord {
 public:
  ClientRecord(uint32_t id, std::experimental::net::ip::tcp::endpoint const&);
  explicit ClientRecord(std::vector<char> const&);
  std::vector<char> to_vector();
  CLIENT_RECORD pack();

  uint32_t const get_id() const;

  static std::vector<char> pack_records(std::vector<ClientRecord> records);
  static std::vector<ClientRecord> unpack_records(std::vector<char> records);

 private:
  uint32_t id_;
  std::experimental::net::ip::tcp::endpoint public_address_;
};
}  // namespace dph
#endif  // INCLUDE_CLIENTRECORD_HPP_
