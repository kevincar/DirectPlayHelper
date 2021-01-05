
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

void waitForStatus(std::shared_ptr<lock_pack> p_lock_pack, std::string const& status) {
  std::unique_lock<std::mutex> status_lock{*p_lock_pack->status_mutex};
  p_lock_pack->status_cv->wait(
      status_lock, [&] { return *p_lock_pack->status == status; });
  status_lock.unlock();
}

void setStatus(std::shared_ptr<lock_pack> p_lock_pack, std::string const& status) {
  { std::lock_guard<std::mutex> status_lock{*p_lock_pack->status_mutex};
    *p_lock_pack->status = status;
  }
  p_lock_pack->status_cv->notify_all();
}

}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_
