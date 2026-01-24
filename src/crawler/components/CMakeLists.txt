add_library(
    components
    STATIC
        frontier/frontier.cpp
        frontier/front_queues.cpp 
        frontier/back_queues.cpp
)

target_link_libraries(
    components
    types
    concurrentqueue::concurrentqueue
)

target_include_directories(components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)