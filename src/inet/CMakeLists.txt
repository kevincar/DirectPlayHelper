set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(inet
	Socket.cpp
	ServiceAddress.cpp
	IPConnection.cpp
	TCPConnection.cpp
	UDPConnection.cpp
	TCPAcceptor.cpp
	MasterConnection.cpp)

target_include_directories(inet BEFORE
	PUBLIC
		../../include
		${G3LOG_INCLUDE_DIRS})
