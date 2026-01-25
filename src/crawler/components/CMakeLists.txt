add_library(
    components
    STATIC
        frontier/frontier.cpp
        frontier/front_queues.cpp 
        frontier/back_queues.cpp
        frontier/frontier_builder.cpp

        frontier/services_implement/sample_front_select.cpp
        frontier/services_implement/sample_prioritizer.cpp
        frontier/services_implement/sample_router.cpp
        frontier/services_implement/sample_back_select.cpp
)

target_link_libraries(
    components
    types
    concurrentqueue::concurrentqueue
)

target_include_directories(components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)