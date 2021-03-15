set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_library(dppl
  ./DPMessage.cpp
  ./DPEnumSessions.cpp
  ./interceptor.cpp
  ./proxy.cpp
  ./DirectPlayServer.cpp
  ./probe/join.cpp
  ./probe/host.cpp
	)

target_include_directories(dppl BEFORE
	PUBLIC
		../../include
		"${g3log_dir}/include"
		"${NETWORK_TS_INCLUDES}"
		)
