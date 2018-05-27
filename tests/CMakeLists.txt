# Make the test executable
add_executable(DPServerTest 
	../src/DPServer.cpp
	../src/ArgParser.cpp
	./src/DPServer_test.cpp
	./src/ArgParser_test.cpp
	./src/inet/Socket_test.cpp
	./src/inet/UDPConnection_test.cpp)

# Includes
target_include_directories(DPServerTest BEFORE
	PUBLIC
		../include
		${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest-release-1.8.0/googletest/include
		${G3LOG_INCLUDE_DIRS})

# Google Testing Library
find_library(GTEST_LIB_DIR 
	NAMES libgtest.a
	HINTS ${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest-release-1.8.0/build/googlemock/gtest/)
message(STATUS "libgtest: ${GTEST_LIB_DIR}")
add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_LIB_DIR})

find_library(GTEST_MAIN_DIR 
	NAMES libgtest_main.a
	HINTS ${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest-release-1.8.0/build/googlemock/gtest/)
message(STATUS "libgtest_main: ${GTEST_MAIN_DIR}")
add_library(gtest_main STATIC IMPORTED)
set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION ${GTEST_MAIN_DIR})

# Link dependencies
target_link_libraries(DPServerTest
	PUBLIC
		g3logger
		inet
		gtest
		gtest_main)
