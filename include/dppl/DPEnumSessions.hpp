#ifndef INCLUDE_DPPL_DPENUMSESSIONS_HPP_
#define INCLUDE_DPPL_DPENUMSESSIONS_HPP_
#include <algorithm>
#include <string>

#include "dppl/DPMessage.hpp"
#include "dppl/dplay.h"
namespace dppl {
class DPEnumSessions : public DPMessage {
 public:
  template <typename T>
  DPEnumSessions(T* data, GUID guidApplication, int flags,
                 std::string password);
};

template <typename T>
DPEnumSessions::DPEnumSessions(T* data, GUID guidApplication, int flags,
                               std::string password)
    : DPMessage(data) {
  this->header()->cbSize = sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS);
  this->header()->token = 0xfab;
  this->set_signature();
  this->header()->command = DPSYS_ENUMSESSIONS;
  this->header()->version = 0xe;

  DPMSG_ENUMSESSIONS* msg = this->message<DPMSG_ENUMSESSIONS>();
  msg->guidApplication = guidApplication;
  msg->dwPasswordOffset = 0;
  msg->dwFlags = flags;

  if (password.size() > 0) {
    std::u16string upassword(password.begin(), password.end());
    int password_byte_length =
        (upassword.size()+1) * sizeof(std::u16string::value_type);
    char* pPassword = reinterpret_cast<char*>(&(*upassword.begin()));

    this->header()->cbSize = sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS) +
                             password_byte_length;
    msg->dwPasswordOffset = sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS) -
                            this->kSignatureOffset;

    std::copy(pPassword, pPassword + password_byte_length,
              this->property_data<char>(msg->dwPasswordOffset));
  }
}
}  // namespace dppl
#endif  // INCLUDE_DPPL_DPENUMSESSIONS_HPP_
