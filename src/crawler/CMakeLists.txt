add_subdirectory(components)
add_subdirectory(types)

add_library(crawler STATIC spider.cpp)

target_include_directories(crawler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)