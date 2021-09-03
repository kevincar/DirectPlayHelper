#include "dp/requestplayerreply.hpp"

namespace dp {
requestplayerreply::requestplayerreply(BYTE* data)
    : message_(reinterpret_cast<DPMSG_REQUESTPLAYERREPLY*>(data)) {
  this->id = this->message_->dwID;
  this->security_desc = this->message_->dpSecurityDesc;
  this->result = this->message_->Results;

  this->load_sspi_provider();
  this->load_capi_provider();
}

std::size_t requestplayerreply::size(void) {
  return sizeof(DPMSG_REQUESTPLAYERREPLY) +
         get_u16string_size(this->sspi_provider) +
         get_u16string_size(this->capi_provider);
}

std::vector<BYTE> requestplayerreply::to_vector(void) {
  std::vector<BYTE> result(this->size(), '\0');
  this->message_ = reinterpret_cast<DPMSG_REQUESTPLAYERREPLY*>(result.data());
  this->message_->dwID = this->id;
  this->message_->dpSecurityDesc = this->security_desc;

  this->message_->dwSSPIProviderOffset =
      this->sspi_provider.size() ? 0x8 + sizeof(DPMSG_REQUESTPLAYERREPLY) : 0;
  this->message_->dwCAPIProviderOffset =
      this->capi_provider.size() ? 0x8 + sizeof(DPMSG_REQUESTPLAYERREPLY) +
                                       get_u16string_size(this->sspi_provider)
                                 : 0;
  this->message_->Results = this->result;

  this->assign_sspi_provider();
  this->assign_capi_provider();
  return result;
}

inline BYTE* requestplayerreply::get_sspi_provider_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         (this->message_->dwSSPIProviderOffset - 0x8);
}

inline BYTE* requestplayerreply::get_capi_provider_ptr(void) {
  return reinterpret_cast<BYTE*>(this->message_) +
         this->message_->dwCAPIProviderOffset - 0x8;
}

void requestplayerreply::load_sspi_provider(void) {
  if (!this->message_->dwSSPIProviderOffset) return;
  char16_t* sspi_provider_ptr =
      reinterpret_cast<char16_t*>(this->get_sspi_provider_ptr());
  std::u16string u16sspi_provider(sspi_provider_ptr);
  this->sspi_provider =
      std::string(u16sspi_provider.begin(), u16sspi_provider.end());
}

void requestplayerreply::load_capi_provider(void) {
  if (!this->message_->dwCAPIProviderOffset) return;
  char16_t* capi_provider_ptr =
      reinterpret_cast<char16_t*>(this->get_capi_provider_ptr());
  std::u16string u16capi_provider(capi_provider_ptr);
  this->capi_provider =
      std::string(u16capi_provider.begin(), u16capi_provider.end());
}

void requestplayerreply::assign_sspi_provider(void) {
  if (!this->sspi_provider.size()) return;
  std::u16string u16sspi_provider(this->sspi_provider.begin(),
                                  this->sspi_provider.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16sspi_provider.data());
  BYTE* end = start + get_u16string_size(this->sspi_provider);
  std::copy(start, end, this->get_sspi_provider_ptr());
}

void requestplayerreply::assign_capi_provider(void) {
  if (!this->capi_provider.size()) return;
  std::u16string u16capi_provider(this->capi_provider.begin(),
                                  this->capi_provider.end());
  BYTE* start = reinterpret_cast<BYTE*>(u16capi_provider.data());
  BYTE* end = start + get_u16string_size(this->capi_provider);
  std::copy(start, end, this->get_capi_provider_ptr());
}
}  // namespace dp
