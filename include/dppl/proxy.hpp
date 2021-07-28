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
class proxy : public std::enable_shared_from_this<proxy> {
 public:
  enum type { host, peer };

  proxy(std::experimental::net::io_context* io_context, type proxy_type,
        std::function<void(DPProxyMessage const&)> dp_callback,
        std::function<void(DPProxyMessage const&)> data_callback);

  void stop();

  std::experimental::net::ip::tcp::endpoint const get_return_addr();
  void set_return_addr(
      std::experimental::net::ip::tcp::endpoint const& app_endpoint);
  void register_player(DPLAYI_SUPERPACKEDPLAYER* player);

  void dp_deliver(std::vector<char> const& data);
  void data_deliver(std::vector<char> const& data);

  DWORD get_system_id() const;
  DWORD get_player_id() const;

  bool operator==(proxy const& rhs);
  bool operator<(proxy const& rhs);
  operator DWORD() const;

 private:
  /* Direct Play Socket Processes */
  void dp_accept();
  void dp_accept_handler(std::error_code const& ec,
                         std::experimental::net::ip::tcp::socket new_socket);
  void dp_receive();
  void dp_receive_handler(std::error_code const& ec,
                          std::size_t bytes_transmitted);
  void dp_receive_requestplayerreply();
  void dp_receive_addforwardrequest_handler();
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

  /* App Data Socket Processes */
  void data_receive();
  void data_receive_handler(std::error_code const& ec,
                            std::size_t bytes_transmitted);
  void data_default_receive_handler();
  void data_send();
  void data_send_handler(std::error_code const& ec,
                         std::size_t bytes_transmitted);

  /* Proxy Attributes */
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
