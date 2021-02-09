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

# CTPL Dependency
ExternalProject_Add(ctpl
	PREFIX "ctpl"
	GIT_REPOSITORY "https://github.com/vit-vit/ctpl"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND "")
ExternalProject_Get_Property(ctpl SOURCE_DIR)
set(ctpl_SOURCE_DIR ${SOURCE_DIR})
message(STATUS "ctpl dir: ${ctpl_SOURCE_DIR}")
# Make the test executable
add_executable("${TEST_PROJECT_NAME}"
	../src/DPServer.cpp
	../src/ArgParser.cpp

	./src/main_test.cpp
	./src/DPServer_test.cpp
	./src/ArgParser_test.cpp
	./src/inet/Socket_test.cpp
	./src/inet/ServiceAddress_test.cpp
	./src/inet/TCPConnection_test.cpp
	./src/inet/UDPConnection_test.cpp
	./src/inet/TCPAcceptor_test.cpp
	./src/inet/MasterConnection_test.cpp

	./src/nathp/Packet_test.cpp
  ./src/nathp/ConnectionRecord_test.cpp
	./src/nathp/ClientRecord_test.cpp
	./src/nathp/Protocol_test.cpp
	)

# Includes
target_include_directories("${TEST_PROJECT_NAME}" BEFORE
	PUBLIC
		../include
		./include
		"${gtest_SOURCE_DIR}/include"
		"${install_dir}/include"
		"${ctpl_SOURCE_DIR}"
		)

# Link dependencies
target_link_libraries("${TEST_PROJECT_NAME}"
	PUBLIC
		g3log
		"${ws2_32}"
		inet
		nathp
		gtest_main)
