#include "dppl/PacketSniffer.hpp"
#include "g3log/g3log.hpp"

namespace dppl {
PacketSniffer::PacketSniffer(
    std::experimental::net::ip::udp::endpoint const& forward_endpoint,
    bool use_localhost)
    : forward_endpoint_(forward_endpoint),
      forward_socket_(io_context_,
                      std::experimental::net::ip::udp::endpoint(
                          std::experimental::net::ip::udp::v4(), 0)) {
  Tins::NetworkInterface dflt = Tins::NetworkInterface::default_interface();
  Tins::NetworkInterface lo(Tins::IPv4Address("127.0.0.1"));
  Tins::NetworkInterface iface = use_localhost ? lo : dflt;

  std::string dst_addr = use_localhost ? "127.0.0.1" : "255.255.255.255";

  this->sniffer_ = std::make_unique<Tins::Sniffer>(iface.name());

  std::string filter_string =
      std::string("ip src ") + iface.addresses().ip_addr.to_string() +
      " and ip dst " + dst_addr + " and udp port " + std::to_string(kPort_);
  this->sniffer_->set_filter(filter_string);

  this->start_sniffing();
}

PacketSniffer::~PacketSniffer() {
  if (this->internal_thread_.get_id() == std::this_thread::get_id()) return;
  this->stop_sniffing();
  this->io_context_.stop();
}

void PacketSniffer::start_sniffing() {
  if (this->internal_thread_.get_id() == std::this_thread::get_id()) return;

  this->internal_thread_ = std::thread([&]() {
    this->sniffer_->sniff_loop(
        Tins::make_sniffer_handler(this, &PacketSniffer::sniff_handler));
  });
}

void PacketSniffer::stop_sniffing() {
  if (this->internal_thread_.get_id() == std::this_thread::get_id()) return;

  this->sniffer_->stop_sniff();
  this->internal_thread_.join();
  return;
}

bool PacketSniffer::sniff_handler(Tins::PDU& pdu) {
  Tins::UDP const& udp = pdu.rfind_pdu<Tins::UDP>();
  Tins::RawPDU const& raw = udp.rfind_pdu<Tins::RawPDU>();

  Tins::RawPDU::payload_type const& payload = raw.payload();
  this->data_.clear();
  this->data_.assign(payload.begin(), payload.end());

  std::error_code ec;
  std::size_t bytes_sent =
      this->forward_socket_.send_to(std::experimental::net::buffer(this->data_),
                                    this->forward_endpoint_, 0, ec);
  if (ec) {
    LOG(WARNING) << "PacketSniffer send error: " << ec.message();
  }
  return true;
}
}  // namespace dppl
