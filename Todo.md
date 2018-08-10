# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [x] Test out the following [Networking Libraries](https://stackoverflow.com/questions/118945/best-c-c-network-library) before continuing to make
  your own. Nope. ASIO requres boost headers, I don't want that. ASE is too
  complex. QT is proprietary.
- [ ] INET Library and namespace
	- [x] Socket
	- [x] ServiceAddress
	- [x] IPConnection
	  - [x] Send
	    - [x] Interface
		- [x] Implementation
		- [x] Test
	  - [x] Recv
	- [x] TCPAcceptor Class
	  - [x] Instance
		- [x] childConnections vector of childConnections
		- [x] connectionHandler function - the function to run when data is
		  available on the childConnection
		- [x] acceptHandler <processHandler> function - the function ran when
		  data is available on the parent TCPAcceptor to check if the
		  incoming connection should be accepted
		- [x] constructor - should take two arguments of shared_ptrs to the
		  process and accept handler functions that will define the behavior
		  or the TCPService
		- [ ] getConnections - return to a vector of constant pointers to the
		  TCPConnections.
		  - [x] inherit std::enable_shared_with_this so that TCPAcceptors can
			access shared pointers to themselves when returning a vector of
			connections with getConnections
	  - [x] Implementation
	  - [x] Testing
	- [ ] MasterConnection
	  - [x] Rename MasterTCPConnection to MasterConnection
	  - [x] Remove the TCPConnection inheritance and make the class it's own
	  - [x] Rename and redefine TCPConnections to Connections as IPConnection
		to support both TCP and UDP
	  - [x] MasterConnection::MasterConnection - should automatically start
		it's connection checking loop even if there are no connections to
		check.
	  - [ ] MasterConnection::checkAllConnectionsForData - get this up and
		running even if no connections are present
		- [ ] MasterConnection::getAllConnections - remove this function. We
		  shouldn't make the code ambiguous. Just manulate the UDPConnection
		  vector and the TCPAcceptor vector directly
	  - [ ] TCP Support
		- [x] MasterConnection::TCPAcceptors - a private vector of TCPAccptors
		  where new TCPAcceptors are pushed to by calling createTCPAcceptor
		- [ ] MasterConnection::createTCPAcceptor - Creates a listening TCP
		  server connection and adds this to the TCPAcceptor list
		- [ ] MasterConnection::removeTCPAcceptor - Destroys a TCPAcceptor and
		  it's child connections
	  - [x] UDP Support
	    - [x] MasterConnection::addConnection - this will be used to decrease
		duplicated code between TCP and udp connection additions
		- [x] Simplify the MasterTCPConnection addition to utilize the new
		addConnection function
		- [x] MasterConnection::addUDP
	  - [ ] Because we want to support multiple processHanders, we should have
		a map of Connections and processHandlers so that different connections
		can be processed differently. But perhaps all TCP will be handled one
		way and UDP will be handled another so there will likely be a large
		difference between the number of connections and number of
		connectionHandlers. They should be shared pointers such that when the
		connection is added, it can check if the pointer already exists and
		then simply refer to that. Additionally, each master function should
		have it's own accept function, the accept function is what adds new
		TCPConnections and so all accepted TCPConnections under the same
		master connection will inherit the same process function.
		UDPConnections however, can do their own thing and do not have master
		connections, although this could simply be implemented, that's not
		their purpose. Instead the master UDP connection should simply handle
		the data that comes in on them and send a response appropriately. At
		least for our case.
	  - [x] Comment out bad code until we can replace it
	  - [ ] MasterConnection::MasterConnection - Constructor - Consider what
		the class should start out with
	  - [ ] What will the class need?
	    - [ ] IPConnections map that maps ID's to connections
		- [ ] processHandler map that maps ID's to processHandlers
		  - Note, masterTCP processHandlers will be those that accept
			incomping connections. Consider whether bool keeps the
			TCPconnection alive or whether that means to accept the connection
			or what?
		- [ ] a map of listening states for all masterTCPs. ONLY if we need
		  them. it is possible that the Listening state is only required for
		  the running thread, in which case this needs to be redefined to
		  `running` state. Do we actually need to keep track of the
		  "listening" state of the masterTCP connections?
	  - [ ] MasterConnection::AddMasterTCP() to add another TCP connection
		that listens for incomming connection. Should take one parameter that
		is std::shared_ptr<std::function> handle to the accept handler function
	- [ ] TCPConnection
	- [ ] UDPConnection
	  - [ ] sendFrom
		- [x] Interface
		- [x] Implementation
		- [ ] Test
	  - [ ] recvFrom
- [ ] DPServer Class
- [ ] Server class
- [ ] Client class
