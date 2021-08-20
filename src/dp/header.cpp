#include "dp/header.hpp"

namespace dp {
header::header(BYTE *data) : header_(reinterpret_cast<DPMSG_HEADER *>(data)) {
  this->size = this->load_size();
  this->token = this->load_token();
  this->sock_addr = this->load_sock_addr();
  this->signature = this->load_signature();
  this->command = this->header_->command;
  this->version = this->header_->version;
}

std::vector<BYTE> header::to_vector(void) {
  DPMSG_HEADER header;
  header.cbSize = this->size;
  header.token = static_cast<DWORD>(this->token);
  header.sockAddr = endpoint_to_sockaddr(this->sock_addr);
  std::copy(this->signature.begin(), this->signature.end(),
            reinterpret_cast<BYTE *>(&header.signature));
  header.command = this->command;
  header.version = this->version;
  BYTE *start = reinterpret_cast<BYTE *>(&header);
  BYTE *end = start + sizeof(DPMSG_HEADER);
  return std::vector<BYTE>(start, end);
}

DWORD header::load_size(void) {
  DWORD first_dword = *reinterpret_cast<DWORD *>(this->header_);
  DWORD size = first_dword & 0x000FFFFF;
  return size;
}

header::Token header::load_token(void) {
  DWORD first_dword = *reinterpret_cast<DWORD *>(this->header_);
  DWORD token_value = (first_dword & 0xFFF00000) >> 20;
  return static_cast<header::Token>(token_value);
}

std::experimental::net::ip::tcp::endpoint header::load_sock_addr(void) {
  sockaddr sockaddr = this->header_->sockAddr;
  return sockaddr_to_endpoint(sockaddr);
}

std::string header::load_signature(void) {
  char *ptr = reinterpret_cast<char *>(&this->header_->signature);
  return std::string(ptr, 4);
}
}  // namespace dp
