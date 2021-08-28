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
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/deps/g3log"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/deps/g3log -DG3_SHARED_LIB=OFF -DUSE_DYNAMIC_LOGGING_LEVELS=ON
	)
ExternalProject_Get_Property(g3log_project install_dir)
set(g3log_dir "${install_dir}")
message(STATUS "g3log installation directory: ${g3log_dir}")
set(g3log_LOC "${g3log_dir}/lib/libg3log.a")
if(MSVC)
	set(g3log_LOC "${g3log_dir}/lib/g3log.lib")
endif()
add_library(g3log STATIC IMPORTED)
set_property(TARGET g3log PROPERTY IMPORTED_LOCATION
	"${g3log_LOC}")
add_dependencies(g3log g3log_project)
set(G3LOG_INCLUDE_DIR ${g3log_dir}/include)

# pcap
if(MSVC)
	message(STATUS "Downloading WinPcap Development Pack")
	set(winpcap_dir ${CMAKE_CURRENT_BINARY_DIR}/deps/winpcap)
	set(winpcapzip ${winpcap_dir}/WpdPack.zip)
	file(DOWNLOAD "https://www.winpcap.org/install/bin/WpdPack_4_1_2.zip" ${winpcapzip})
	file(ARCHIVE_EXTRACT INPUT ${winpcapzip} DESTINATION ${winpcap_dir})
	set(wpdpack ${winpcap_dir}/WpdPack)
	set(WINCMAKEARGS -DLIBTINS_BUILD_TESTS=OFF -DPCAP_ROOT_DIR=${wpdpack})
	set(PCAP_LIBRARY_PATH ${wpdpack}/Lib)
	if(CMAKE_CL_64)
		set(PCAP_LIBRARY_PATH ${PCAP_LIBRARY_PATH}/x64)
	endif()
	set(PCAP_INCLUDE_DIR ${wpdpack}/Include)
	set(PCAP_LIBRARY ${PCAP_LIBRARY_PATH}/wpcap.lib)
	set(PCAP_PACKET_LIBRARY ${PCAP_LIBRARY_PATH}/Packet.lib)
	add_library(wpcap STATIC IMPORTED)
	set_property(TARGET wpcap PROPERTY IMPORTED_LOCATION ${PCAP_LIBRARY})
	add_library(packet STATIC IMPORTED)
	set_property(TARGET packet PROPERTY IMPORTED_LOCATION ${PCAP_PACKET_LIBRARY})
	add_library(pcap INTERFACE IMPORTED)
	set_property(TARGET pcap PROPERTY INTERFACE_LINK_LIBRARIES wpcap packet)
endif()

# libtins
ExternalProject_Add(libtins_project
	GIT_REPOSITORY https://github.com/mfontanini/libtins
	GIT_TAG v4.3
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/deps/libtins"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/deps/libtins -DLIBTINS_ENABLE_CXX11=1 -DTINS_STATIC=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_BUILD_SHARED=0 ${WINCMAKEARGS}
	)
ExternalProject_Get_Property(libtins_project install_dir)
set(libtins_dir "${install_dir}")
message(STATUS "libtins installation directory: ${libtins_dir}")
set(libtins_LOC "${libtins_dir}/lib/libtins.a")
if(MSVC)
	set(libtins_LOC "${libtins_dir}/lib/tins.lib")
	set(libtins_deps pcap Ws2_32.lib Iphlpapi.lib)
else()
	set(libtins_deps pcap)
endif()
add_library(libtins STATIC IMPORTED)
set_property(TARGET libtins PROPERTY IMPORTED_LOCATION
	"${libtins_LOC}")
add_dependencies(libtins libtins_project)
set(LIBTINS_INCLUDE_DIR ${libtins_dir}/include)
set(LIBTINS_LIBRARY libtins ${libtins_deps})
add_compile_definitions(TINS_STATIC=1)

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

# argparse
execute_process(
  COMMAND git clone https://github.com/p-ranav/argparse "${CMAKE_CURRENT_BINARY_DIR}/deps/argparse"
  )
set(ARGPARSE_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/deps/argparse/include")

#####################################################################
#                                                                   #
#                           PROGRAM                                 #
#                                                                   #
#####################################################################

# build the app
add_executable("${PROJECT_NAME}"
	src/main.cpp
  #src/Message.cpp
  #src/ClientRecord.cpp
  #src/Client.cpp
  #src/Server.cpp
  #src/DirectPlayHelper.cpp
	)

# Add dependency targets to this main target
add_dependencies("${PROJECT_NAME}" g3log libtins)

# Includes
target_include_directories("${PROJECT_NAME}" BEFORE
	PUBLIC
		include
		${G3LOG_INCLUDE_DIR}
		${NETWORK_TS_INCLUDES}
    ${ARGPARSE_INCLUDES}
		)

# build the libraries
add_subdirectory(src/dp)
add_subdirectory(src/dppl)
#add_subdirectory(src/nathp)

# Link dependencies
if(NOT WIN32)
	set(PTHREAD_LIBRARY "pthread")
endif()

target_link_libraries("${PROJECT_NAME}"
	PUBLIC
		#nathp
		#dppl
    dp
		${LIBTINS_LIBRARY}
		g3log
		${PTHREAD_LIB}
		)

# Oh yeah, and add the tests
add_subdirectory(tests)


#############################
#       INSTALL             #
#############################

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
