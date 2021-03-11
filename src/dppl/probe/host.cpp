#include "dppl/dp.hpp"
#include "dppl/dplay.h"
#include "dppl/probe/host.hpp"
#include "experimental/net"

namespace dppl {
namespace probe {
host::host(std::experimental::net::io_context* io_context, GUID app_guid)
    : app_guid_(app_guid),
      io_context_(io_context),
      timer_(*io_context, std::chrono::seconds(10)),
      broadcast_socket_(*io_context,
                        std::experimental::net::ip::udp::endpoint(
                            std::experimental::net::ip::udp::v4(), 0)),
      broadcast_endpoint_(std::experimental::net::ip::address_v4::broadcast(),
                          kPort_),
      dp_acceptor_(*io_context_, std::experimental::net::ip::tcp::endpoint(
                                     std::experimental::net::ip::tcp::v4(), 0)),
      dp_socket_(*io_context_, std::experimental::net::ip::tcp::endpoint(
                                   std::experimental::net::ip::tcp::v4(), 0)) {
  this->broadcast_socket_.set_option(
      std::experimental::net::socket_base::broadcast(true));

  this->accept_dp_connections();
}

void host::accept_dp_connections() {
  auto handler = std::bind(&host::accept_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->dp_acceptor_.async_accept(handler);
}

void host::accept_handler(std::error_code const& ec,
                          std::experimental::net::ip::tcp::socket socket) {
  if (!ec) {
    std::cout << "Accepted :)" << std::endl;
    this->dp_socket_ = std::move(socket);
    this->read_dp_message();
  } else {
    std::cout << "Accept Error: " << ec.message() << std::endl;
  }
  this->accept_dp_connections();
}

void host::read_dp_message() {
  auto handler = std::bind(&host::read_handler, this, std::placeholders::_1,
                           std::placeholders::_2);
  this->buf_.resize(kBufSize_, '\0');
  std::cout << "Set for reading on: "
            << this->dp_socket_.local_endpoint().address() << ":"
            << this->dp_socket_.local_endpoint().port() << std::endl;
  this->dp_socket_.async_receive(
      std::experimental::net::buffer(this->buf_),
      handler
      );
}

void host::read_handler(std::error_code const& ec, std::size_t bytes_received) {
  if (!ec) {
    DPMessage response(&(*this->buf_.begin()));
    DPMSG_ENUMSESSIONSREPLY* msg = response.message<DPMSG_ENUMSESSIONSREPLY>();
    int name_len = response.header()->cbSize - sizeof(DPMSG_HEADER) - sizeof(DPMSG_ENUMSESSIONSREPLY);
    char* start = reinterpret_cast<char*>(msg->szSessionName);
    char* end = start+name_len;
    std::u16string wsessionname(start, end);
    std::string session_name(wsessionname.begin(), wsessionname.end());
    std::cout << session_name << std::endl;
  } else {
    std::cout << "Read Error: " << ec.message() << std::endl;
  }
}

void host::prepare_packet() {
  this->buf_.clear();
  this->buf_.resize(kBufSize_, '\0');

  int i = 0;
  std::string signature = "play";

  sockaddr_in* ret_addr = reinterpret_cast<sockaddr_in*>(
      this->dp_acceptor_.local_endpoint().data());
  DPEnumSessions message(&(*this->buf_.begin()), this->app_guid_,
                         ENUMSESSIONSFLAGS::allsessions |
                             ENUMSESSIONSFLAGS::passwordprotectedsessions |
                             ENUMSESSIONSFLAGS::unksessions,
                         "");
  message.set_return_addr(*ret_addr);

  this->buf_.resize(message.header()->cbSize);
}
}  // namespace probe
}  // namespace dppl
