set(TEST_PROJECT_NAME "${PROJECT_NAME}Test")

# GTEST Dependency
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# Make the test executable
add_executable("${TEST_PROJECT_NAME}"
  ../src/Message.cpp
  ../src/ClientRecord.cpp
  ../src/Client.cpp
  ../src/Server.cpp

  # Should be listed in order of dependecies
  ./src/dppl/hardware_test.cpp
  ./src/dppl/AppSimulator.cpp
  ./src/dppl/dplay_size_test.cpp
  ./src/dppl/PacketSniffer_test.cpp
  ./src/dppl/DPProxyMessage_test.cpp
  ./src/dppl/DPSuperPackedPlayer_test.cpp
  ./src/dppl/DirectPlayServer_test.cpp
  ./src/dppl/proxy_test.cpp
  ./src/dppl/interceptor_test.cpp
  ./src/Message_test.cpp
  ./src/ClientRecord_test.cpp
  ./src/Client_test.cpp
  ./src/Server_test.cpp
	./src/main_test.cpp

	#./src/nathp/Packet_test.cpp
	#./src/nathp/ClientRecord_test.cpp
	#./src/nathp/Server_test.cpp

	)

# Includes
target_include_directories("${TEST_PROJECT_NAME}" BEFORE
	PUBLIC
		../include
		./include
		${PCAP_INCLUDE_DIR}
		${G3LOG_INCLUDE_DIR}
		${LIBTINS_INCLUDE_DIR}
		"${gtest_SOURCE_DIR}/include"
		"${NETWORK_TS_INCLUDES}"
		)

# Link dependencies
target_link_libraries("${TEST_PROJECT_NAME}"
	PUBLIC
		#nathp
		dppl
		gtest_main
		g3log
		${LIBTINS_LIBRARY}
		)
