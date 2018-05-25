add_library(inet
	Socket.cpp)

target_include_directories(inet BEFORE
	PUBLIC
		../../include)
