#include "dp/header.hpp"

namespace dp {
header::header(std::vector<BYTE> *message_data)
    : message_data_(message_data),
      data_(reinterpret_cast<BYTE *>(&(*message_data->begin()))),
      header_(reinterpret_cast<DPMSG_HEADER *>(data_)) {}

DWORD header::get_cb_size(void) {
  DWORD first_dword = *reinterpret_cast<DWORD *>(this->data_);
  DWORD size = first_dword & 0x000FFFFF;
  return size;
}

void header::set_cb_size(std::size_t size) {
  DWORD *first_dword = reinterpret_cast<DWORD *>(this->data_);
  DWORD current_token = *first_dword & 0xFFF00000;
  DWORD limited_size = static_cast<DWORD>(size) & 0x000FFFFF;
  DWORD new_first_dword = current_token | limited_size;
  *first_dword = new_first_dword;
  if (this->message_data_->size() != size) {
    this->message_data_->resize(size);
  }
}

header::token header::get_token(void) {
  DWORD first_dword = *reinterpret_cast<DWORD *>(this->data_);
  DWORD token_value = (first_dword & 0xFFF00000) >> 20;
  return static_cast<header::token>(token_value);
}

void header::set_token(header::token new_token) {
  DWORD *first_dword = reinterpret_cast<DWORD *>(this->data_);
  DWORD current_size = *first_dword & 0x000FFFFF;
  DWORD token_dword = static_cast<DWORD>(new_token);
  DWORD new_first_dword = (token_dword << 20) | current_size;
  *first_dword = new_first_dword;
}

std::experimental::net::ip::tcp::endpoint header::get_sock_addr(void) {
  sockaddr sockaddr = this->header_->sockAddr;
  return sockaddr_to_endpoint(sockaddr);
}

void header::set_sock_addr(
    std::experimental::net::ip::tcp::endpoint const &endpoint) {
  this->header_->sockAddr = endpoint_to_sockaddr(endpoint);
}

std::string header::get_signature(void) {
  char *ptr = reinterpret_cast<char *>(&this->header_->signature);
  return std::string(ptr, 4);
}

void header::set_signature(std::string const& signature) {
  char const *start = reinterpret_cast<char const *>(&(*signature.begin()));
  char const *end = start + 4;
  char *dest = reinterpret_cast<char *>(&this->header_->signature);
  std::copy(start, end, dest);
}

WORD header::get_command(void) {
  return this->header_->command;
}

void header::set_command(WORD command) {
  this->header_->command = command;
}

WORD header::get_version(void) {
  return this->header_->version;
}

void header::set_version(WORD version) {
  this->header_->version = version;
}
}  // namespace dp
