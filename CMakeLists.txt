cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME DirectPlayHelper)
include(ExternalProject)

project("${PROJECT_NAME}" VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# G3Logger the package cmake script doesn't add the library we'll do that here
#find_package(g3logger REQUIRED CONFIG)
#add_library(g3logger STATIC IMPORTED)
#set_target_properties(g3logger PROPERTIES IMPORTED_LOCATION ${G3LOG_LIBRARIES})
#message(STATUS "g3logger libraries: ${G3LOG_INCLUDE_DIRS}")
ExternalProject_Add(g3log_project
	GIT_REPOSITORY https://github.com/KjellKod/g3log
	GIT_TAG master
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/g3log"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/g3log -DG3_SHARED_LIB=OFF
	)
ExternalProject_Get_Property(g3log_project install_dir)
message(STATUS "g3log installation directory: ${install_dir}")
set(g3log_LOC "${install_dir}/lib/libg3logger.a")
if(MSVC)
	set(g3log_LOC "${install_dir}/lib/g3logger.lib")
endif()
add_library(g3log STATIC IMPORTED)
set_property(TARGET g3log PROPERTY IMPORTED_LOCATION
	"${g3log_LOC}")
add_dependencies(g3log g3log_project)

# build the app
add_executable("${PROJECT_NAME}"
	src/main.cpp
	#src/DPServer.cpp
	#src/ArgParser.cpp
	)

# Includes
target_include_directories("${PROJECT_NAME}" BEFORE
	PUBLIC
		include
		"${install_dir}/include"
		)

# build the inet library
#add_subdirectory(src/inet)

# Link dependencies
target_link_libraries("${PROJECT_NAME}"
	PUBLIC
		g3log
		#inet
		)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
