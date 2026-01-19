add_library(
    components
    STATIC
        frontier/frontier.cpp
)

target_link_libraries(
    components
    types
)

target_include_directories(components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)