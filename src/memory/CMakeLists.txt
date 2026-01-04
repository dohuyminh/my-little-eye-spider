add_library(memory STATIC arena.cpp)

target_include_directories(networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)