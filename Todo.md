# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [ ] INET Library
	- [x] Socket
	- [x] ServiceAddress
	- [x] IPConnection
	- [ ] TCPConnection
	- [ ] UDPConnection
	- [ ] MasterTCPConnection
		- [x] Accept
		- [x] Connect
		- [ ] Before we go forward. We need to consider our design. We
			shouldn't simply create one thread for every connection. That
			will bog us down. So take time to consider if we can simply do
			everything in one thread… We'll need to make a branch for this
			so we can revert changes
			- [x] Create a branch called connection-redesign
			- [x] Design a single thread connection
				- [x] Remove connection maps, and make vectors again…
				- [x] IPConnection::operator int()
					- [x] Interface
					- [x] Implementation
					- [x] Test
				- [x] getLargestSocket
					- [x] Interface
					- [x] Implementation
				- [x] checkAllConnectionsForData Function
					- [x] Interface
					- [x] Implementation
				- [x] getNumConnections - We need this so that we can create
				  a successful test case for listenForIncominConnections. The
				  Server side should test to see if the number of connections
				  is greater than 0
				  	- [x] Interface
					- [x] Implementation
					- [x] Test
				- [x] listenForIncominConnections Function
					- [x] Interface
					- [x] Implementation
					- [x] Test
				- [x] Reformat the connectionProcessHandlerFunc to return
				  a boolean value such that the checkAllConnectionsForData
				  function will then remove the connection when it returns
				  false
				- [x] removeConnection - For removing previously accepted
				  connections
					- [x] Interface
					- [x] Implementation
					- [x] Test
				- Layout
					- Basic steps
						- Listen for a knock at the door or for incoming letters
						- If It's a knock at the door
							- Open the door when some one is there
							- Decide whether to let them in
							- Once they're in add them to the list of connections
						- If it's a letter
							- Process the message

	  - [ ] Send
	  - [ ] Recv
	  - [ ] sendFrom
	  - [ ] recvFrom
- [ ] DPServer Class
- [ ] Server class
- [ ] Client class
