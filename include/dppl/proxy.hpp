#ifndef INCLUDE_DPPL_PROXY_HPP_
#define INCLUDE_DPPL_PROXY_HPP_
#define TXCR "\u001b[31m"
#define TXCG "\u001b[32m"
#define TXCY "\u001b[33m"
#define TXFB "\u001b[1m"
#define TXRS "\u001b[0m"

#include <experimental/net>
#include <vector>

#include "dppl/DPMessage.hpp"
#include "dppl/DPProxyMessage.hpp"

#define POLOG(X) LOG(X) << TXCY << TXFB
#define PILOG(X) LOG(X) << TXCG << TXFB
#define PELOG TXRS

namespace dppl {
class DPProxyMessage;
// Normally a DirectPlay application will connect to other DirectPlay
// application directly through a network address and port. Without settting up
// port forwarding NATS essentially block communication. This is because the
// `DPMSG_HEADER` that is sent between computers has a return address that is
// set by the local computer and assumes other computers on the network can
// connect to it. The `proxy` class then sits in as proxy for conenctions
// outside of the network. When a remote peer attempts to connect or send a
// message to the local DirectPlay Application, a local proxy is set up to
// officiate as this remote peer. All messages send by this remote peer are
// are captured by delgated proxy and the messages are then altered so that the
// return address points back to our proxy. This way the local DirectPlay
// application will send messages that are intended for the remote peer to this
// proxy instead so that the message can be sent back out to the internet to
// the approprate peer connection.
class proxy : public std::enable_shared_from_this<proxy> {
 public:
  // Whether the proxy represents a host or peer conneection
  enum type { host, peer };

  // DirectPlay uses two forms of communication, one over TCP to setup a
  // connection between peers, and another to transfer data once the connection
  // is established. Data comming in from the local DirectPlay application
  // during connection setup (typically on TCP 2300-2350) is processed and
  // forwarded ot the `dp_callback` that should be defined by the class that
  // owns the proxy. THe `data_callback` is the same but for data that is
  // transfered once the connection is established (typically UDP 2350)
  proxy(std::experimental::net::io_context* io_context, type proxy_type,
        std::function<void(DPProxyMessage)> dp_callback,
        std::function<void(DPProxyMessage)> data_callback);

  void stop();

  std::experimental::net::ip::tcp::endpoint const get_return_addr();
  void set_return_addr(
      std::experimental::net::ip::tcp::endpoint const& app_endpoint);

  // When information either coming from a remote host or from our local
  // hosting DirectPlay application contains information about other players we
  // need to know about, this function registers those players so that other
  // proxies can be set up to establish connections with them when ready
  void register_player(DPLAYI_SUPERPACKEDPLAYER* player);

  // the `_deliver` functions are used by the owning class to send incoming
  // remote messages to the local DirectPlay application
  void dp_deliver(DPProxyMessage data);
  void data_deliver(DPProxyMessage data);

  DWORD get_client_id() const;
  DWORD get_system_id() const;
  DWORD get_player_id() const;
  DPProxyEndpointIDs get_ids() const;

  bool operator==(proxy const& rhs);
  bool operator<(proxy const& rhs);
  operator DWORD() const;

 private:
  // Direct Play Socket Processes
  void dp_accept();
  void dp_accept_handler(std::error_code const& ec,
                         std::experimental::net::ip::tcp::socket new_socket);
  void dp_receive();
  void dp_receive_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);
  void dp_receive_requestplayerreply();
  void dp_receive_addforwardrequest_handler();
  void dp_receive_superenumplayersreply_handler();
  void dp_default_receive_handler();
  void dp_send();
  void dp_assert_connection();
  void dp_send_enumsession_handler();
  void dp_send_enumsessionreply_handler();
  void dp_send_requestplayerid();
  void dp_send_addforwardrequest();
  void dp_send_createplayer_handler();
  void dp_default_send_handler();
  void dp_receipt_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);

  // App Data Socket Processes
  void data_receive();
  void data_receive_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted);
  void data_default_receive_handler();
  void data_send();
  void data_send_handler(std::error_code const& ec,
                         std::size_t bytes_transmitted);

  bool validate_message(DPProxyMessage const& message);

  // Proxy Attributes
  DWORD client_id_ = 0;
  DWORD system_id_ = 0;
  DWORD player_id_ = 0;
  int recent_request_flags_ = -1;
  type proxy_type_;

  static int const kBufSize_ = 512;
  std::vector<char> dp_recv_buf_;
  std::vector<char> dp_send_buf_;
  std::vector<char> data_recv_buf_;
  std::vector<char> data_send_buf_;
  std::experimental::net::io_context* io_context_;
  std::experimental::net::ip::tcp::endpoint app_dp_endpoint_;
  std::experimental::net::ip::tcp::acceptor dp_acceptor_;
  std::experimental::net::ip::tcp::socket dp_send_socket_;
  std::experimental::net::ip::tcp::socket dp_recv_socket_;
  std::experimental::net::ip::udp::socket dpsrvr_socket_;
  std::experimental::net::ip::udp::socket data_socket_;

  std::function<void(DPProxyMessage const&)> dp_callback_;
  std::function<void(DPProxyMessage const&)> data_callback_;
};
}  // namespace dppl
#endif  // INCLUDE_DPPL_PROXY_HPP_
