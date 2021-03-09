cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME DirectPlayHelper)
include(ExternalProject)

project("${PROJECT_NAME}" VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#####################################################################
#                                                                   #
#                           DEPENDENCIES                            #
#                                                                   #
#####################################################################

# G3LOG
ExternalProject_Add(g3log_project
	GIT_REPOSITORY https://github.com/KjellKod/g3log
	GIT_TAG 1.3.4
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/g3log"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/g3log -DG3_SHARED_LIB=OFF -DUSE_DYNAMIC_LOGGING_LEVELS=ON
	)
ExternalProject_Get_Property(g3log_project install_dir)
message(STATUS "g3log installation directory: ${install_dir}")
set(g3log_LOC "${install_dir}/lib/libg3log.a")
if(MSVC)
	set(g3log_LOC "${install_dir}/lib/g3log.lib")
endif()
add_library(g3log STATIC IMPORTED)
set_property(TARGET g3log PROPERTY IMPORTED_LOCATION
	"${g3log_LOC}")
add_dependencies(g3log g3log_project)

# experimental/net
execute_process(
  COMMAND git clone https://github.com/chriskohlhoff/networking-ts-impl "${CMAKE_CURRENT_BINARY_DIR}/deps/networking-ts-impl"
  )
set(NETWORK_TS_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/deps/networking-ts-impl/include")
macro(get_WIN32_WINNT version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

		set(${version} "0x${ver}")
    endif()
endmacro()
if(WIN32)
	get_WIN32_WINNT(ver)
	add_definitions(-D_WIN32_WINNT=${ver})
endif()

#####################################################################
#                                                                   #
#                           PROGRAM                                 #
#                                                                   #
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
        ${NETWORK_TS_INCLUDES}
		)

# build the libraries
add_subdirectory(src/nathp)

# Link dependencies
if(NOT WIN32)
	set(PTHREAD_LIB "pthread")
else()
	set(PTHREAD_LIB "")
endif()

target_link_libraries("${PROJECT_NAME}"
	PUBLIC
		g3log
    "${PTHREAD_LIB}"
		nathp
		)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
