cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME DirectPlayHelper)
include(ExternalProject)

project("${PROJECT_NAME}" VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#####################################################################
#																	#
#						DEPENDENCIES								#
#																	#
#####################################################################

ExternalProject_Add(g3log_project
	GIT_REPOSITORY https://github.com/KjellKod/g3log
	GIT_TAG master
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/g3log"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/g3log -DG3_SHARED_LIB=OFF -DUSE_DYNAMIC_LOGGING_LEVELS=ON
	)
ExternalProject_Get_Property(g3log_project install_dir)
message(STATUS "g3log installation directory: ${install_dir}")
set(g3log_LOC "${install_dir}/lib/libg3log.a")
if(MSVC)
	#set(g3log_LOC "${install_dir}/lib/g3logger.lib")
	set(g3log_LOC "${install_dir}/lib/g3log.lib")
endif()
add_library(g3log STATIC IMPORTED)
set_property(TARGET g3log PROPERTY IMPORTED_LOCATION
	"${g3log_LOC}")
add_dependencies(g3log g3log_project)

#####################################################################
#																	#
#							PROGRAM									#
#																	#
#####################################################################

# build the app
add_executable("${PROJECT_NAME}"
	src/main.cpp
	src/DPServer.cpp
	src/ArgParser.cpp
	)

# Includes
target_include_directories("${PROJECT_NAME}" BEFORE
	PUBLIC
		include
		"${install_dir}/include"
		)

# build the libraries
add_subdirectory(src/inet)
add_subdirectory(src/nathp)

# Link dependencies
if(WIN32)
	set(ws2_32 ws2_32.lib)
endif()
target_link_libraries("${PROJECT_NAME}"
	PUBLIC
		g3log
		"${ws2_32}"
		inet
		nathp
		)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
