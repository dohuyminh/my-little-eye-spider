add_library(
    components
    STATIC
        worker/worker.cpp
        worker/worker_pool.cpp
)

target_link_libraries(
    components
    PUBLIC
        types
        concurrentqueue::concurrentqueue
        nlohmann_json::nlohmann_json
        services
)

target_include_directories(components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
