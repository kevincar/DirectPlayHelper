
#ifndef TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_
#define TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_

#include <mutex>
#include <string>

namespace nathp {
namespace asset {
class lock_pack {
 public:
  std::string* status;
  std::mutex* status_mutex;
  std::condition_variable* status_cv;
};
}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_
