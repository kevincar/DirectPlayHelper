set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
include(CheckIncludeFiles)
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../include/nathp/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/nathp/config.hpp")

add_library(nathp
	Server.cpp
	)

target_include_directories(nathp BEFORE
	PUBLIC
		../../include
		${install_dir}/include
		#"${CMAKE_CURRENT_BINARY_DIR}/include" # For config.hpp
		)
