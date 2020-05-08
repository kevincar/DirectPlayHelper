cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME DirectPlayHelper)
project("${PROJECT_NAME}" VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# G3Logger the package cmake script doesn't add the library we'll do that here
#find_package(g3logger REQUIRED CONFIG)
#add_library(g3logger STATIC IMPORTED)
#set_target_properties(g3logger PROPERTIES IMPORTED_LOCATION ${G3LOG_LIBRARIES})
#message(STATUS "g3logger libraries: ${G3LOG_INCLUDE_DIRS}")

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
		#${G3LOG_INCLUDE_DIRS}
		)

# build the inet library
#add_subdirectory(src/inet)

# Link dependencies
target_link_libraries("${PROJECT_NAME}"
	PUBLIC
		#g3logger
		#inet
		)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
