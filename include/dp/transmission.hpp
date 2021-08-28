#ifndef INCLUDE_DP_TRANSMISSION_HPP_
#define INCLUDE_DP_TRANSMISSION_HPP_

#include <memory>
#include <vector>

#include "dp/types.h"
#include "dp/message.hpp"

namespace dp {
class transmission {
 public:
  transmission(void);
  explicit transmission(std::shared_ptr<std::vector<BYTE>> data);
  explicit transmission(std::vector<BYTE> data);
  std::vector<BYTE> const& to_vector(void) const;
  bool is_dp_message(void) const;

  std::shared_ptr<message> msg;

 private:
  mutable std::shared_ptr<std::vector<BYTE>> data_;
};
}  // namespace dp
#endif  // INCLUDE_DP_TRANSMISSION_HPP_