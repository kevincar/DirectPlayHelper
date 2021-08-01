#include "dppl/DPProxyMessage.hpp"

namespace dppl {
DPProxyMessage::DPProxyMessage(DPProxyMessage const& other)
    : data_(other.data_) {}

std::vector<char> DPProxyMessage::to_vector() const { return this->data_; }

std::vector<char> DPProxyMessage::get_dp_msg_data() const {
  std::vector<char> dp_msg;
  auto start = this->data_.begin() + sizeof(DPPROXYMSG);
  auto end = this->data_.end();
  dp_msg.assign(start, end);
  return dp_msg;
}

DPMessage DPProxyMessage::get_dp_msg() {
  char* data = reinterpret_cast<char*>(&(*this->data_.begin()));
  data += sizeof(DPPROXYMSG);
  return DPMessage(data);
}

DPProxyEndpointIDs DPProxyMessage::get_to_ids() const {
  DPPROXYMSG const* msg =
      reinterpret_cast<DPPROXYMSG const*>(&(*this->data_.begin()));
  return {msg->to.clientID, msg->to.systemID, msg->to.playerID};
}

DPProxyEndpointIDs DPProxyMessage::get_from_ids() const {
  DPPROXYMSG const* msg =
      reinterpret_cast<DPPROXYMSG const*>(&(*this->data_.begin()));
  return {msg->from.clientID, msg->from.systemID, msg->from.playerID};
}

void DPProxyMessage::set_to_ids(DPProxyEndpointIDs toIDs) {
  DPPROXYMSG* msg = reinterpret_cast<DPPROXYMSG*>(&(*this->data_.begin()));
  msg->to = toIDs;
}

void DPProxyMessage::set_to_ids(proxy const& to) {
  this->set_to_ids(this->proxy_to_ids(to));
}

void DPProxyMessage::set_from_ids(DPProxyEndpointIDs fromIDs) {
  DPPROXYMSG* msg = reinterpret_cast<DPPROXYMSG*>(&(*this->data_.begin()));
  msg->from = fromIDs;
}

void DPProxyMessage::set_from_ids(proxy const& from) {
  this->set_from_ids(this->proxy_to_ids(from));
}

DPProxyEndpointIDs DPProxyMessage::proxy_to_ids(proxy const& p) {
  DWORD client_id = p.get_client_id();
  DWORD system_id = p.get_system_id();
  DWORD player_id = p.get_player_id();
  DPProxyEndpointIDs ids = {client_id, system_id, player_id};
  return ids;
}
}  // namespace dppl
