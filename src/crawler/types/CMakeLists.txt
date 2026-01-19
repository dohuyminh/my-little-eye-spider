add_library(
    types 
    STATIC 
        url.cpp)

target_include_directories(types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)