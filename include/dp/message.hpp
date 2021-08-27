#ifndef INCLUDE_DP_MESSAGE_HPP_
#define INCLUDE_DP_MESSAGE_HPP_

#include <memory>
#include <vector>

#include "dp/base_message.hpp"
#include "dp/header.hpp"
#include "dp/types.h"

namespace dp {
class message {
 public:
  explicit message(BYTE* data);
  std::size_t size(void);
  std::vector<BYTE> to_vector(void);

  header header;
  std::shared_ptr<base_message> msg;

 private:
  BYTE* data_;
};
}  // namespace dp
#endif  // INCLUDE_DP_MESSAGE_HPP_
