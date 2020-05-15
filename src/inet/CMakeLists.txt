set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
include(CheckIncludeFiles)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("errno.h" HAVE_ERRNO_H)
check_include_files("sys/socket.h" HAVE_SOCKET_H)
check_include_files("winsock2.h" HAVE_WINSOCK2_H)
check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
check_include_files("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_files("ws2tcpip.h" HAVE_WS2TCPIP_H)
check_include_files("sys/select.h" HAVE_SELECT_H)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../include/inet/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/inet/config.hpp")

add_library(inet
	Socket.cpp
	ServiceAddress.cpp
	IPConnection.cpp
	TCPConnection.cpp
	UDPConnection.cpp
	TCPAcceptor.cpp
	MasterConnection.cpp
	)

target_include_directories(inet BEFORE
	PUBLIC
		../../include
		${install_dir}/include
		"${CMAKE_CURRENT_BINARY_DIR}/include" # For config.hpp
		)
