add_library(inet
	Socket.cpp
	IPConnection.cpp
	TCPConnection.cpp
	UDPConnection.cpp)

target_include_directories(inet BEFORE
	PUBLIC
		../../include)
