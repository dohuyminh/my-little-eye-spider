add_library(
    types 
    STATIC 
        url.cpp
        runnable.cpp)

target_link_libraries(
    types
        services
)

target_include_directories(types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)