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
	  - [ ] Rename MasterTCPConnection to MasterConnection
	  - [ ] Remove the TCPConnection inheritance and make the class it's own
	  - [ ] Rename and redefine TCPConnections to Connections as IPConnection
		to support both TCP and UDP
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
