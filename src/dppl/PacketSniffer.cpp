#include "dppl/PacketSniffer.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
PacketSniffer::PacketSniffer(std::experimental::net::io_context* io_context,
                             std::function<bool(std::vector<char>)> forward)
    : io_context_(io_context), forward_(forward) {
  LOG(DEBUG) << "Creating Sniffer";
  Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
  this->sniffer_ = std::make_unique<Tins::Sniffer>(iface.name());

  std::string filter_string = std::string("ip src ") +
                              iface.addresses().ip_addr.to_string() +
                              " and udp port " + std::to_string(kPort_);
  this->sniffer_->set_filter(filter_string);

  this->sniff();
}

void PacketSniffer::sniff() {
  auto handler = std::bind(&PacketSniffer::sniff_handler, this);

  std::experimental::net::defer(*this->io_context_, handler);
}

void PacketSniffer::sniff_handler() {
  Tins::PDU const* pdu = this->sniffer_->next_packet();
  Tins::UDP const* udp = pdu->find_pdu<Tins::UDP>();
  if (udp == nullptr) return this->sniff();

  Tins::RawPDU const* raw = udp->find_pdu<Tins::RawPDU>();
  if (raw == nullptr) return this->sniff();

  Tins::RawPDU::payload_type const& payload = raw->payload();
  std::vector<char> data(payload.begin(), payload.end());

  this->forward_(data);
  this->sniff();
}
}  // namespace dppl
