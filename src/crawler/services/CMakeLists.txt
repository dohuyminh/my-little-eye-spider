add_library(
    services 
    STATIC 
        url.cpp
        thread_pool.cpp
        html_downloader.cpp
        curl_response.cpp
        curl_handler.cpp
)

target_link_libraries(
    services
        types
        re2::re2
        libidn2::libidn2
        libpsl::libpsl
        CURL::libcurl
)

target_include_directories(services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)