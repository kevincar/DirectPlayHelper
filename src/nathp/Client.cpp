
#include "nathp/Client.hpp"

#include <sstream>

#include <g3log/g3log.hpp>

#include "nathp/Packet.hpp"

namespace nathp {
Client::Client(std::string serverIPAddress, int port, bool start) {
  this->serverAddress = serverIPAddress + ":" + std::to_string(port);
  this->clearProcStat();

  // Establish a connection
  if (start) {
    this->connect();
  }

  // Setup a thread to handle communication with the server
  this->ch = std::bind(&Client::connectionHandler, this, std::placeholders::_1);
}

void Client::connect(void) {
  bool connected = false;
  bool keepTrying = true;
  int nRetries = this->nConnectionRetries;
  while (!connected && keepTrying) {
    // TODO(@kevincar): Without this sleep call the IPConenction failes... why?
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    int result = this->serverConnection.connect(this->serverAddress);
    if (result != 0) {
      LOG(WARNING) << "Failed to connect! result = " << std::to_string(result)
                   << " errno - " << std::to_string(ERRORCODE);
    } else {
      connected = true;
    }

    if (nRetries != -1) {
      nRetries--;
      keepTrying = nRetries > 0;
    }
  }

  if (!connected) {
    LOG(FATAL) << "Exhausted connection retries!";
    throw std::runtime_error("Failed to connect client to server");
  }

  // LOG(DEBUG) << "Client connected: FD = " <<
  // static_cast<int>(this->serverConnection) << " | srcAddr = " <<
  // this->serverConnection.getAddressString() << " destAddr = " <<
  // this->serverConnection.getDestAddressString();
  this->serverConnection.startHandlerProcess(this->ch);
  this->id = this->requestClientID();
  this->serverConnection.setPublicAddress(this->requestPublicAddress());
  return;
}

unsigned int Client::requestClientID(void) const noexcept {
  unsigned int result = -1;
  Packet packet;
  packet.senderID = -1;
  packet.recipientID = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::initClientID;
  int sendResult = packet.sendVia(this->serverConnection);
  if (sendResult == -1) {
    LOG(WARNING) << "Client failed to request the client ID | errno: "
                 << std::to_string(ERRORCODE);
    return result;
  }

  std::unique_lock<std::mutex> proc_lock{this->proc_mutex};
  this->proc_cv.wait(proc_lock, [&] { return this->proc_stat[packet.msg]; });
  packet.setPayload(this->proc_rslt);
  result = *reinterpret_cast<unsigned int*>(packet.payload.data());
  return result;
}

std::string Client::requestPublicAddress(void) const noexcept {
  std::string result;
  Packet packet;
  packet.senderID = this->id;
  packet.recipientID = 0;
  packet.type = Packet::Type::request;
  packet.msg = Packet::Message::initPublicAddress;
  int sendResult = packet.sendVia(this->serverConnection);
  if (sendResult == -1) {
    LOG(WARNING)
        << "Client failed to request the client public address | errno: "
        << std::to_string(ERRORCODE);
    return result;
  }

  std::unique_lock<std::mutex> proc_lock{this->proc_mutex};
  this->proc_cv.wait(proc_lock, [&] { return this->proc_stat[packet.msg]; });
  packet.setPayload(this->proc_rslt);
  result.assign(packet.payload.begin(), packet.payload.end());
  return result;
}

std::vector<nathp::ClientRecord> Client::getClientList(void) const noexcept {
  // LOG(DEBUG) << "Client: Sending packet request for getClientList";
  std::vector<ClientRecord> result;
  Packet packet;
  int sendResult = packet.sendVia(this->serverConnection);
  if (sendResult == -1) {
    LOG(WARNING) << "Client failed to request getClientList | errno: "
                 << std::to_string(ERRORCODE);
  }

  std::unique_lock<std::mutex> proc_lock{this->proc_mutex};
  this->proc_cv.wait(proc_lock, [&] { return this->proc_stat[packet.msg]; });
  packet.setPayload(this->proc_rslt);
  for (auto it = packet.payload.begin(); it != packet.payload.end();) {
    ClientRecord cr;
    _ClientRecord* _cr = reinterpret_cast<_ClientRecord*>(&(*it));
    unsigned int size = sizeof(_ClientRecord) + _cr->addressLen;
    cr.setData(reinterpret_cast<unsigned char const*>(_cr), size);
    if (cr.address != this->serverConnection.getAddressString())
      result.push_back(cr);
    it += size;
  }
  // LOG(DEBUG) << "Client: sent packet request for getClientList " <<
  // result.size();
  return result;
}

bool Client::connectToPeer(ClientRecord const& clientRecord) const noexcept {
  // 1. Set up a new inet::TCPConnection to connect to the server
  // 2. Use the same connection handler as the server since the protocols
  // shouldn't be any different
  // 3. Send a request
  return true;
}

bool Client::connectionHandler(inet::IPConnection const& connection) {
  // LOG(DEBUG) << "Client connection handler!";
  unsigned int const buffer_size = 1024 * 4;
  std::vector<char> buffer(buffer_size, '\0');
  if (!connection.isDataReady(5.0)) return true;

  // LOG(DEBUG) << "Client Connection Handler: Received Data";
  int retval = connection.recv(buffer.data(), buffer_size);
  if (retval == -1) {
    LOG(WARNING) << "Client failed to receive data | errno: "
                 << std::to_string(ERRORCODE);
    return true;
  } else if (retval == 0) {
    // LOG(DEBUG) << "Client connection closed";
    return false;
  }

  // Do any NATHP processing like changes state flags etc.
  Packet packet;
  packet.setData((unsigned char const*)buffer.data(), buffer.size());
  // LOG(DEBUG) << "Size received: " << packet.size();

  // if there is data to send to the processHandler get it and send it
  this->processPacket(packet);
  return true;
}

void Client::processPacket(Packet const& packet) const noexcept {
  this->clearProcStat();

  std::lock_guard<std::mutex> proc_lock{this->proc_mutex};
  if (packet.type != Packet::Type::response) {
    LOG(WARNING) << "Client received a NATHP request packet and should have "
                    "received a response packet! "
                 << packet.type << " ≠ " << Packet::Type::response;
    return;
  }

  switch (packet.msg) {
    case Packet::Message::initClientID:
    case Packet::Message::initPublicAddress:
    case Packet::Message::getClientList: {
      packet.getPayload(&this->proc_rslt);
      this->proc_stat[packet.msg] = true;
      this->proc_cv.notify_all();
      break;
    }
    default:
      LOG(WARNING)
          << "Client received a NATHP packet with an unrecognized command";
  }
  return;
}

void Client::clearProcStat(void) const noexcept {
  std::lock_guard<std::mutex> proc_lock{this->proc_mutex};
  for (int i = Packet::Message::getClientList;
       i < Packet::Message::getClientList + 1; i++) {
    Packet::Message j = static_cast<Packet::Message>(i);
    this->proc_stat[j] = false;
  }
  return;
}
}  // namespace nathp
