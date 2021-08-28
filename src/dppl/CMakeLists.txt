set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_library(dppl
  ./PacketSniffer.cpp
  ./message.cpp
  ./DirectPlayServer.cpp
  ./proxy.cpp
  #./interceptor.cpp
	)

target_link_libraries(dppl
	PUBLIC
		${LIBTINS_LIBRARY}
		g3log
		)


target_include_directories(dppl BEFORE
	PUBLIC
		../../include
		${PCAP_INCLUDE_DIR}
		${G3LOG_INCLUDE_DIR}
		${LIBTINS_INCLUDE_DIR}
		${NETWORK_TS_INCLUDES}
		)
