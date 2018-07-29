# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [ ] INET Library
	- [x] Socket
	- [x] ServiceAddress
	- [x] IPConnection
	  - [x] Send
	    - [x] Interface
		- [x] Implementation
		- [x] Test
	  - [x] Recv
	- [ ] MasterConnection
	  - [x] Rename MasterTCPConnection to MasterConnection
	  - [x] Remove the TCPConnection inheritance and make the class it's own
	  - [x] Rename and redefine TCPConnections to Connections as IPConnection
		to support both TCP and UDP
	  - [x] MasterConnection::MasterConnection - should automatically start
		it's connection checking loop even if there are no connections to
		check.
	  - [x] MasterConnection::checkAllConnectionsForData - get this up and
		running even if no connections are present
	  - [ ] TCP Support
		- [x] MasterConnection::masterTCPList - this mapped list has
		  masterTCP connection IDs as the key and a vector of child connection
		  IDs as the value
		  - [x] Should be added to with calls to createMasterTCP and when new
			connections are accepted
		  - [x] Should have stuff removed from it when masterTCPConnections
			are removed or when child connections end
		- [x] MasterConnection::MasterChildProcHandlers - is a map member that
		  maps the connectionID's of the masterTCP connection to the pointer
		  of the processHandler that will be assigned to the master's children
	    - [x] MasterConnection::createMasterTCP - processHandlers for
		  masterTCP returning false means don't add the connection, returning
		  true means add the connection. Where as for other standard
		  connection true will mean keep the connection, false will mean
		  remove the connection. This function should take two
		  processHandlers, one for it's on process, and another for all child
		  processes. Also, we should consider how we can link child
		  connections to parent connections.
		- [x] MasterConnection::isMasterTCP - this is mainly for the
		  checkAllConnectionsForData function. So that when it receives a true
		  response, it accepts the connection
		- [x] MasterConnection::acceptConnection - to accept connections from
		  masterTCP sockets
		- [x] MasterConnection::removeMasterTCP
		- [ ] MasterConnection::acceptConnection 
		  - [x] should also include the connID of the master, so that the
			child can be added to the masterList
		  - [ ] TCPConnection.accept is never being call. We don't have an
			instance of the connection to accept. figure out a way to
			implement this with our current design!
			- [ ] The best way will be to reformat acceptConnection to simply
			  take the masterID. Use that to obtain the TCPConnection
			  instance, call the accept function on the master instance to
			  obtain an instance of the child, and work the child from there
			  - [ ] Apparently TCPConnections do not currently define accept.
				Perhaps this should be added to the TCPConnection class.
				Investigate the BSD function accept(2) do determine whether
				accept has any baring on UDPConnections. 
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
