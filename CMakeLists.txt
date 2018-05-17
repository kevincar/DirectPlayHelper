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

target_include_directories(DPServer 
	PUBLIC
		include
	)
