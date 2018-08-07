
#ifndef INET_MASTER_CONNECTION_HPP
#define INET_MASTER_CONNECTION_HPP

#include <map>
#include <vector>
#include "inet/TCPConnection.hpp"
#include "inet/UDPConnection.hpp"
#include "inet/TCPAcceptor.hpp"

namespace inet
{
	class MasterConnection
	{
		public:
			typedef std::function<bool (IPConnection const&)> ProcessHandler;
			
			MasterConnection(void);
			~MasterConnection(void);
			bool isListening(void) const;
			unsigned long getNumConnections(void) const;
			unsigned int createTCPAcceptor(std::shared_ptr<TCPAcceptor::AcceptHandler> const& pAcceptPH, std::shared_ptr<TCPAcceptor::ProcessHandler> const& pChildPH);
			void removeMasterTCP(unsigned int connID);
			unsigned int createUDPConnection(std::shared_ptr<ProcessHandler> const& pPH);
			void removeUDPConnection(unsigned int connID);
			void acceptConnection(unsigned int masterID, std::shared_ptr<TCPConnection> const& newTCPConnection);
			void stopListening(void);
			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

		private:
			std::thread listeningThread;
			std::mutex listeningThread_mutex;

			bool listening = false;
			mutable std::mutex listening_mutex;

			std::vector<std::shared_ptr<TCPAcceptor>> acceptors;
			mutable std::mutex acceptor_mutex;

			std::vector<UDPConnection> udpConnections;
			mutable std::mutex udp_mutex;

			std::map<unsigned int, std::shared_ptr<ProcessHandler>> processHandlers;
			mutable std::mutex proc_mutex;

			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
			void startListening();

			std::unique_ptr<std::vector<IPConnection const*> const> getAllConnections(void) const;

			bool checkAllConnectionsForData(double timeout);
			int getLargestSocket(void) const;
			//unsigned int addConnection(std::shared_ptr<IPConnection> const& pIPconn, std::shared_ptr<processHandler> const& pPH);
			void removeConnection(unsigned int connID);
			bool isConnMasterTCP(unsigned int connID) const;
	};
}

#endif /* INET_MASTER_CONNECTION_HPP */
