# Make the test executable
add_executable(DPServerTest 
	../src/DPServer.cpp
	./src/DPServer_test.cpp)

# Includes
target_include_directories(DPServerTest BEFORE
	PUBLIC
		../include
		${G3LOG_INCLUDE_DIRS})

# Google Testing Library
find_library(GTEST_LIB_DIR libgtest.a)
add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_LIB_DIR})
find_library(GTEST_MAIN_DIR libgtest_main.a)
add_library(gtest_main STATIC IMPORTED)
set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION ${GTEST_MAIN_DIR})

# Link dependencies
target_link_libraries(DPServerTest
	PUBLIC
		g3logger
		gtest
		gtest_main)

