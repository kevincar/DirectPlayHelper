
#ifndef TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_
#define TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_

#include <mutex>
#include <string>

namespace nathp {
namespace asset {

template <typename T>
class lock_var {
 public:
  lock_var(T const& val);
  void wait(T const& val);
  void operator=(T const& other);
 private:
  T value;
  std::mutex value_mutex;
  std::condition_variable value_cv;
};

typedef lock_var<std::string> lock_string;

template<typename T>
lock_var<T>::lock_var(T const& val) : value(val) {}

template <typename T>
void lock_var<T>::wait(T const& val) {
  std::unique_lock<std::mutex> value_lock(this->value_mutex);
  this->value_cv.wait(
      value_lock, [&] { return this->value == val; });
  value_lock.unlock();
}

template <typename T>
void lock_var<T>::operator=(T const& other) {
  { std::lock_guard<std::mutex> value_lock{this->value_mutex};
    this->value = other;
  }
  this->value_cv.notify_all();
}

}  // namespace asset
}  // namespace nathp

#endif  // TESTS_INCLUDE_NATHP_ASSETS_PROTOCOL_HPP_
