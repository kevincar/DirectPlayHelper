
#ifndef INET_MASTER_CONNECTION_HPP
#define INET_MASTER_CONNECTION_HPP

#include <map>
#include <vector>
#include "inet/TCPConnection.hpp"
#include "inet/UDPConnection.hpp"

namespace inet
{
	class MasterConnection
	{
		public:
			//typedef std::function<bool (std::shared_ptr<TCPConnection>&)> newConnectionAcceptHandlerFunc;
			typedef std::function<bool (std::shared_ptr<IPConnection>&)> processHandler;
			
			MasterConnection(void);
			~MasterConnection(void);
			bool isListening(void) const;
			unsigned long getNumConnections(void) const;
			unsigned int createMasterTCP(std::shared_ptr<processHandler>& pPH);
			void removeMasterTCP(unsigned int connID);
			unsigned int createUDPConnection(std::shared_ptr<processHandler>& pPH);
			void acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection);
			void removeConnection(std::shared_ptr<TCPConnection>& conn);
			//void listenForIncomingConnections(newConnectionAcceptHandlerFunc const& ncaHandler, connectionProcessHandlerFunc const& cpHandler);
			void stopListening(void);
			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

		private:
			std::thread listeningThread;
			std::mutex listeningThread_mutex;
			std::map<unsigned int, std::shared_ptr<IPConnection>> connections;
			mutable std::mutex conn_mutex;
			std::map<unsigned int, std::shared_ptr<processHandler>> processHandlers;
			std::mutex proc_mutex;
			std::vector<unsigned int> masterTCPList;
			std::mutex masterTCPList_mutex;
			bool listening = false;
			mutable std::mutex listening_mutex;
			//newConnectionAcceptHandlerFunc newConnectionAcceptHandler;

			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
			void startListening();
			bool checkAllConnectionsForData(double timeout);
			int getLargestSocket(void) const;
			unsigned int addConnection(std::shared_ptr<IPConnection>& pIPconn, std::shared_ptr<processHandler>& pPH);
	};
}

#endif /* INET_MASTER_CONNECTION_HPP */
