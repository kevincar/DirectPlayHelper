# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------

[x] - Hello World
[x] - DPServer Class
[ ] - Server class
[ ] - Client class

[ ] - INET Library
	[ ] - Socket
	[ ] - ServiceAddress
	[ ] - IPConnection
	[ ] - TCPConnection
	[ ] - UDPConnection
	[ ] - MasterTCPConnection
		[ ] - Accept
		[ ] - Connect
		[ ] - Before we go forward. We need to consider our design. We
			shouldn't simply create one thread for every connection. That
			will bog us down. So take time to consider if we can simply do
			everything in one thread… We'll need to make a branch for this
			so we can revert changes
			[ ] - Create a branch called connection-redesign
			[ ] - Consider if we should make more than one thread, if so, how
				many for server and clients, and what will their purpose be?
				[ ] - Design a single thread connection
				[ ] - MasterTCPConnection destructor needs to handle shutdown
					of child connection threads Create a stopConnection
					function to stop a connection Destructor should look
					through and call stopConnection on all

	  [ ] - Send
	  [ ] - Recv
	  [ ] - sendFrom
	  [ ] - recvFrom
