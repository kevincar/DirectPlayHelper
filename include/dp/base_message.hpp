#ifndef INCLUDE_DP_BASE_MESSAGE_HPP_
#define INCLUDE_DP_BASE_MESSAGE_HPP_

#include <vector>

#include "dp/types.h"

namespace dp {
class base_message {
 public:
  explicit base_message(BYTE* data);
  base_message(void);
  virtual std::size_t size(void) = 0;
  virtual std::vector<BYTE> to_vector(void) = 0;

 private:
  BYTE* data_;
};
}  // namespace dp
#endif  // INCLUDE_DP_BASE_MESSAGE_HPP_
