add_subdirectory(components)
add_subdirectory(types)

add_library(crawler STATIC spider.cpp)

target_link_libraries(crawler PUBLIC components types)

target_include_directories(crawler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)