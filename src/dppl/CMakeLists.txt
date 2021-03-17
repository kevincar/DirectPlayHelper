set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_library(dppl
  ./DPMessage.cpp
  ./DPEnumSessions.cpp
  ./PacketSniffer.cpp
  ./interceptor.cpp
  ./proxy.cpp
  ./DirectPlayServer.cpp
  ./probe/join.cpp
  ./probe/host.cpp
	)

add_dependencies(dppl g3log libtins)

target_include_directories(dppl BEFORE
	PUBLIC
		../../include
		${PCAP_INCLUDE_DIR}
		${G3LOG_INCLUDE_DIR}
		${LIBTINS_INCLUDE_DIR}
		${NETWORK_TS_INCLUDES}
		)
