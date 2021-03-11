#ifndef INCLUDE_DPPL_DPMESSAGE_HPP_
#define INCLUDE_DPPL_DPMESSAGE_HPP_
#include "dppl/dplay.h"
namespace dppl {
class DPMessage {
 public:
  template <typename T>
  explicit DPMessage(T* message_data);

  DPMSG_HEADER* header();

  void set_return_addr(sockaddr_in const& addr);

  void set_signature();

  template <typename T>
  T* message();

  template <typename T>
  T* property_data(int offset);

  static int const kSignatureOffset = sizeof(DWORD) + sizeof(sockaddr_in);

 private:
  char* data_;
};

template <typename T>
DPMessage::DPMessage(T* message_data)
    : data_(reinterpret_cast<char*>(message_data)){};

template <typename T>
T* DPMessage::message() {
  return reinterpret_cast<T*>(this->data_ + sizeof(DPMSG_HEADER));
}

template <typename T>
T* DPMessage::property_data(int offset) {
  return reinterpret_cast<T*>(this->data_ + this->kSignatureOffset + offset);
}

}  // namespace dppl

#endif  // INCLUDE_DPPL_DPMESSAGE_HPP_
