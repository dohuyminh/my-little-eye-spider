add_library(networking STATIC listener.cpp client_socket.cpp)

target_include_directories(networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)