# At LEAST 2.8 but newer is better
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(DPServer VERSION 0.1 LANGUAGES CXX)

#target_compile_features(DPServer PRIVATE cxx_range_for)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# G3Logger the package cmake script doesn't add the library we'll do that here
find_package(g3logger REQUIRED CONFIG)
add_library(g3logger STATIC IMPORTED)
set_target_properties(g3logger PROPERTIES IMPORTED_LOCATION ${G3LOG_LIBRARIES})
message(STATUS "g3logger libraries: ${G3LOG_INCLUDE_DIRS}")

# build the app
add_executable(DPServer
	src/main.cpp
	src/DPServer.cpp
	src/ArgParser.cpp
	)

# Includes
target_include_directories(DPServer BEFORE
	PUBLIC
		include
		${G3LOG_INCLUDE_DIRS})

# Link dependencies
target_link_libraries(DPServer
	PUBLIC
		g3logger)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

install(TARGETS DPServer DESTINATION bin)
