add_library(
    services 
    STATIC 
        url.cpp
)

target_link_libraries(
    services
        re2::re2
        libidn2::libidn2
)

target_include_directories(services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)