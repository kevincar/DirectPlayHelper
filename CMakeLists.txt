# At LEAST 2.8 but newer is better
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(DPServer VERSION 0.1 LANGUAGES CXX)

#target_compile_features(DPServer PRIVATE cxx_range_for)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(DPServer
	src/main.cpp
	src/DPServer.cpp
	)

find_library(g3log_location NAMES libg3logger.dylib)
message(STATUS ${g3log_location})

target_include_directories(DPServer 
	PUBLIC
		include
	)

add_library(g3log STATIC IMPORTED)
set_target_properties(g3log PROPERTIES IMPORTED_LOCATION ${g3log_location})
target_link_libraries(DPServer
	PUBLIC
		g3log)
