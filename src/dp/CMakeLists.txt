set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_library(dp
  ./base_message.cpp
  ./header.cpp
  ./sessiondesc.cpp
  ./packedplayer.cpp
  ./playerinfomask.cpp
  ./superpackedplayer.cpp
  ./enumsessionsreply.cpp
  ./enumsessions.cpp
  ./enumplayersreply.cpp
  ./requestplayerid.cpp
  ./requestgroupid.cpp
  ./requestplayerreply.cpp
  ./createplayer.cpp
  ./addforwardrequest.cpp
  ./superenumplayersreply.cpp
  ./message.cpp
  ./transmission.cpp
  )

target_link_libraries(dp
  PUBLIC
    ${LIBTINS_LIBRARY}
    g3log
    )

target_include_directories(dp BEFORE
  PUBLIC
    ../../include
    ${PCAP_INCLUDE_DIR}
    ${G3LOG_INCLUDE_DIR}
    ${LIBTINS_INCLUDE_DIR}
    ${NETWORK_TS_INCLUDES}
    )
