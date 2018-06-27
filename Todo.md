# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [x] DPServer Class
- [ ] Server class
- [ ] Client class
- [ ] INET Library

	- -[ ] Socket
	- -[ ] ServiceAddress
	- -[ ] IPConnection
	- -[ ] TCPConnection
	- -[ ] UDPConnection
	- -[ ] MasterTCPConnection
		- -[ ] Accept
		- -[ ] Connect
		- -[ ] Before we go forward. We need to consider our design. We
			shouldn't simply create one thread for every connection. That
			will bog us down. So take time to consider if we can simply do
			everything in one thread… We'll need to make a branch for this
			so we can revert changes
			- -[ ] Create a branch called connection-redesign
			- -[ ] Consider if we should make more than one thread, if so, how
				many for server and clients, and what will their purpose be?
				- -[ ] Design a single thread connection
				- -[ ] MasterTCPConnection destructor needs to handle shutdown
					of child connection threads Create a stopConnection
					function to stop a connection Destructor should look
					through and call stopConnection on all
					- -[ ] Before we go forward. We need to consider our
					  design. We shouldn't simply create one thread for every
					  connection. That will bog us down. So take time to
					  consider if we can simply do everything in one thread…
					  We'll need to make a branch for this so we can revert
					  changes.
						- -[ ] Create a branch called connection-redesign
						- -[ ] Consider if we should make more than one
						  thread, if so, how many for server and clients, and
						  what will their purpose be?
						- -[ ] Design a single thread connection
							- -[ ] We need the following
								- -[ ] A processHandler for our listening
								  socket that adds accpected connections to
								  the MasterTCPConnection's vector of
								  connections
								- -[ ] We need a function that check for
								  incoming information on all sockets and
								  return a vector of bools.
								- -[ ] We then need another function to check
								  on those bools to then call the
								  processHandler for that connection
								- -[ ] We need a map for both connections and
								  processHandler. The key should be a unique
								  identifier that should exist in both
								- -[ ] We need to research how to genereat
								  Unique ID's so that when a new
								  connection/process Is added that we can
								  assign it an ID
	  - -[ ] Send
	  - -[ ] Recv
	  - -[ ] sendFrom
	  - -[ ] recvFrom
