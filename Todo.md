# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [x] DPServer Class
- [ ] Server class
- [ ] Client class
- [ ] INET Library

	- -[x] Socket
	- -[x] ServiceAddress
	- -[x] IPConnection
	- -[ ] TCPConnection
	- -[ ] UDPConnection
	- -[ ] MasterTCPConnection
		- -[x] Accept
		- -[x] Connect
		- -[ ] Before we go forward. We need to consider our design. We
			shouldn't simply create one thread for every connection. That
			will bog us down. So take time to consider if we can simply do
			everything in one thread… We'll need to make a branch for this
			so we can revert changes
			- -[x] Create a branch called connection-redesign
			- -[ ] Consider if we should make more than one thread, if so, how
				many for server and clients, and what will their purpose be?
				- -[ ] Design a single thread connection
				- -[ ] MasterTCPConnection destructor needs to handle shutdown
					of child connection threads Create a stopConnection
					function to stop a connection Destructor should look
					through and call stopConnection on all
				- -[x] Remove connection maps, and make vectors again…
				- -[ ] IPConnection::operator int()
					- -[x] Interface
					- -[x] Implementation
					- -[ ] Test
				- -[ ] getLargestSocket
					- -[x] Interface
					- -[ ] Implementation
				- -[ ] checkAllConnectionsForData Function
					- -[x] Interface
					- -[ ] Implementation
				- -[ ] listenForIncominConnections Function
					- -[ ] Interface
					- -[ ] Implementation
					- -[ ] Test
				- Layout
					- Basic steps
						- Listen for a knock at the door or for incoming letters
						- If It's a knock at the door
							- Open the door when some one is there
							- Decide whether to let them in
							- Once they're in add them to the list of connections
						- If it's a letter
							- Process the message

	  - -[ ] Send
	  - -[ ] Recv
	  - -[ ] sendFrom
	  - -[ ] recvFrom
