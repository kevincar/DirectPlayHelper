
#include "nathp/Client.hpp"

#include <sstream>
#include <utility>
#include <g3log/g3log.hpp>

#include "nathp/Packet.hpp"
#include "nathp/type.hpp"

namespace nathp {
Client::Client(std::string server_ip_address, int port, bool start) {
  this->server_address = server_ip_address + ":" + std::to_string(port);
  this->clearProcResponseData();

  // Establish a connection
  if (start) {
    this->connect();
  }

  // Setup a thread to handle communication with the server
  this->server_connection_handler =
      std::bind(&Client::connectionHandler, this,
          std::placeholders::_1);
}

void Client::connect(void) {
  bool connected = false;
  bool keep_trying = true;
  int n_retries = this->reconnection_attempts;
  while (!connected && keep_trying) {
    // TODO(@kevincar): Without this sleep call the IPConenction failes... why?
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    int result = this->server_connection.connect(this->server_address);
    if (result != 0) {
      LOG(WARNING) << "Failed to connect! result = " << std::to_string(result)
                   << " errno - " << std::to_string(ERRORCODE);
    } else {
      connected = true;
    }

    if (n_retries != -1) {
      n_retries--;
      keep_trying = n_retries > 0;
    }
  }

  if (!connected) {
    LOG(FATAL) << "Exhausted connection retries!";
    throw std::runtime_error("Failed to connect client to server");
  }

  this->server_connection.startHandlerProcess(this->server_connection_handler);
  this->id = this->requestClientID();
  this->server_connection.setPublicAddress(this->requestPublicAddress());
  this->requestRegisterPrivateAddress();
  return;
}

ClientRecord Client::getClientRecord(void) const {
  ClientRecord client_record(this->id);
  client_record.public_address =
      this->server_connection.getPublicAddressString();
  client_record.private_address = this->server_connection.getAddressString();
  return client_record;
}

std::vector<nathp::ClientRecord> Client::requestClientList(
    void) const noexcept {
  LOG(DEBUG) << "Client " << this->id
             << " sending packet request for getClientList";
  std::vector<ClientRecord> result;
  Packet packet;
  packet.sender_id = this->id;
  packet.recipient_id = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::getClientList;
  int sendResult = this->sendPacketTo(packet, this->server_connection);
  if (sendResult == -1) {
    LOG(WARNING) << "Client failed to request getClientList | errno: "
                 << std::to_string(ERRORCODE);
  }

  this->awaitResponse(&packet);
  for (auto it = packet.payload.begin(); it != packet.payload.end();
       it += sizeof(_ClientRecord)) {
    uint8_t const* client_record_data =
        reinterpret_cast<uint8_t const*>(&(*it));
    ClientRecord client_record(client_record_data);
    result.push_back(client_record);
  }
  LOG(DEBUG) << "Client: sent packet request for getClientList "
             << result.size();
  return result;
}

bool Client::connectToPeer(ClientRecord const& client_record) noexcept {
  LOG(DEBUG) << "connectToPeer";
  bool holepunch_succeeded = this->createHolepunch(type::UDP, client_record);
  if (holepunch_succeeded) return true;

  // bool relay_succeeded = this->requestRelayStatus();
  return true;
}

bool Client::createHolepunch(type holepunch_type,
                             ClientRecord const& client_record) {
    bool result = false;
    switch(holepunch_type) {
      case type::UDP: {
        result = this->requestUDPHolepunch(client_record);
      } break;
      case type::TCP: {
        // result = this->requestTCPHolepunch(client_record);
      } break;
      default:
        LOG(WARNING) << "type supplied to createHolepunch is unknown";
    }
    return result;
}

bool Client::connectionHandler(inet::IPConnection const& connection) {
  LOG(DEBUG) << "Running connection handler for client ID " << this->id;
  if (!connection.isDataReady(5.0)) return true;

  LOG(DEBUG) << "Client ID " << this->id << " received data";
  unsigned int const kBufSize = 1024 * 4;
  std::vector<uint8_t> buffer(kBufSize, '\0');
  int retval =
      connection.recv(reinterpret_cast<char*>(buffer.data()), kBufSize);
  if (retval == -1) {
    LOG(WARNING) << "Client failed to receive data | errno: "
                 << std::to_string(ERRORCODE);
    return true;
  } else if (retval == 0) {
    LOG(DEBUG) << "Client ID " << this->id << " connection closed";
    return false;
  }

  Packet packet{buffer};
  LOG(DEBUG) << "Recieved a packet " << packet.size() << " bytes long";

  if (packet.type == Packet::Type::request) {
    this->processRequestPacket(packet);
  } else {
    this->processResponsePacket(packet);
  }
  return true;
}

void Client::processRequestPacket(Packet const& packet) noexcept {
  switch (packet.msg) {
    case Packet::Message::udpHolepunch: {
      ClientRecord peer_record (packet.getPayload<std::vector<uint8_t>>());
      this->initHolepunch(peer_record);
      break;
    }
    default:
      LOG(WARNING)
        << "Client received a NATHP packet with an uncrecongized request";
  }
}

void Client::processResponsePacket(Packet const& packet) const noexcept {
  this->clearProcResponseData();

  std::lock_guard<std::mutex> proc_response_lock{this->proc_response_mutex};

  switch (packet.msg) {
    case Packet::Message::getClientId:
    case Packet::Message::getPublicAddress:
    case Packet::Message::registerPrivateAddress:
    case Packet::Message::getClientList: 
    case Packet::Message::udpHolepunch: {
      this->proc_response_data[packet.msg] = packet.getPayload<std::vector<uint8_t>>();
      this->proc_response_ready[packet.msg] = true;
      this->proc_response_cv.notify_all();
      break;
    }
    default:
      LOG(WARNING)
          << "Client received a NATHP packet with an unrecognized command";
  }
  return;
}

void Client::clearProcResponseData(void) const noexcept {
  std::lock_guard<std::mutex> proc_lock{this->proc_response_mutex};
  for (int i = Packet::Message::getClientId;
       i < Packet::Message::getClientList + 1; i++) {
    Packet::Message j = static_cast<Packet::Message>(i);
    this->proc_response_ready[j] = false;
  }
  return;
}

int Client::sendPacketTo(Packet const& packet,
                         inet::IPConnection const& conn) const {
  char const* data = reinterpret_cast<char const*>(packet.data());
  unsigned int const kSize = packet.size();
  return conn.send(data, kSize);
}

void Client::awaitResponse(Packet* packet) const noexcept {
  std::unique_lock<std::mutex> proc_response_lock{this->proc_response_mutex};
  this->proc_response_cv.wait(proc_response_lock, [&] {
    return this->proc_response_ready[packet->msg];
  });
  packet->setPayload(this->proc_response_data[packet->msg]);
}

unsigned int Client::requestClientID(void) const noexcept {
  LOG(DEBUG) << "Requesting Client ID";
  unsigned int result = -1;

  Packet packet;
  packet.sender_id = -1;
  packet.recipient_id = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::getClientId;
  int sendResult = this->sendPacketTo(packet, this->server_connection);
  if (sendResult == -1) {
    LOG(WARNING) << "Client failed to request the client ID | errno: "
                 << std::to_string(ERRORCODE);
    return result;
  }

  this->awaitResponse(&packet);
  result = *reinterpret_cast<unsigned int*>(packet.payload.data());
  LOG(DEBUG) << "Client received ID of " << result;
  return result;
}

std::string Client::requestPublicAddress(void) const noexcept {
  std::string result;

  Packet packet;
  packet.sender_id = this->id;
  packet.recipient_id = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::getPublicAddress;
  int send_result = this->sendPacketTo(packet, this->server_connection);
  if (send_result == -1) {
    LOG(WARNING)
        << "Client failed to request the client public address | errno: "
        << std::to_string(ERRORCODE);
    return result;
  }

  this->awaitResponse(&packet);
  result.assign(packet.payload.begin(), packet.payload.end());
  LOG(DEBUG) << "Client " << this->id << " received public address " << result;
  return result;
}

void Client::requestRegisterPrivateAddress(void) const noexcept {
  Packet packet;
  packet.sender_id = this->id;
  packet.recipient_id = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::registerPrivateAddress;
  packet.setPayload(this->server_connection.getAddressString());

  int send_result = this->sendPacketTo(packet, this->server_connection);
  if (send_result == -1) {
    LOG(WARNING) << "Client failed to register the private address | errno: "
                 << std::to_string(ERRORCODE);
    return;
  }

  this->awaitResponse(&packet);
  LOG(DEBUG) << "Client " << this->id << " registered private address";
}

bool Client::requestUDPHolepunch(ClientRecord const& client_record) {
  Packet packet;
  packet.sender_id = this->id;
  packet.recipient_id = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::udpHolepunch;
  packet.setPayload(client_record.id);

  int send_result = this->sendPacketTo(packet, this->server_connection);
  if (send_result == -1) {
    LOG(WARNING) << "Client failed to send a request to UDP hole punch| errno: "
                 << std::to_string(ERRORCODE);
    return false;
  }

  this->awaitResponse(&packet);
  std::string response = packet.getPayload<std::string>();
  if (response != "OK") return false;
  return this->initHolepunch(client_record);
}

bool Client::initHolepunch(ClientRecord const& peer_record) {
  std::unique_ptr<inet::UDPConnection> p_peer_connection =
    std::make_unique<inet::UDPConnection>();

  //std::lock_guard lock(this->peer_conn_mutex);
  //this->peer_connections.push_back(std::move(p_peer_connection));
  return true;
}
}  // namespace nathp
