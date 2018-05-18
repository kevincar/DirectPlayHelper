# At LEAST 2.8 but newer is better
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(DPServer VERSION 0.1 LANGUAGES CXX)

#target_compile_features(DPServer PRIVATE cxx_range_for)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# we should probably be using g3log package
#find_library(g3log_location libg3logger.dylib)
#find_path(G3LOG_ROOT_DIR
	#NAMES include/g3log/g3log.hpp
	#)
#message(STATUS ${G3LOG_ROOT_DIR})

#add_library(g3log STATIC IMPORTED)
#set_target_properties(g3log PROPERTIES IMPORTED_LOCATION ${g3log_location})

# G3Logger the package cmake script doesn't add the library we'll do that here
find_package(g3logger REQUIRED CONFIG)
add_library(g3logger STATIC IMPORTED)
set_target_properties(g3logger PROPERTIES IMPORTED_LOCATION ${G3LOG_LIBRARIES})

# build the app
add_executable(DPServer
	src/main.cpp
	src/DPServer.cpp
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
