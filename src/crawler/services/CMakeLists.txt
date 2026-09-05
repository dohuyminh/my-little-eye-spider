add_library(
    services
    STATIC
        url.cpp
        thread_pool.cpp
        coroutine_thread_pool.cpp
        http_get_handler.cpp
        html_parser.cpp
        curl_response.cpp
        curl_handler.cpp
        time_wheel.cpp
)

target_compile_definitions(services PRIVATE BOOST_ASIO_HAS_CO_AWAIT)

target_link_libraries(
    services
    PRIVATE
        types
    PUBLIC
        CURL::libcurl
        lexbor::lexbor_static
        concurrentqueue::concurrentqueue
        nlohmann_json::nlohmann_json
        re2::re2
        libidn2::libidn2
        libpsl::libpsl
        Boost::headers
)

target_include_directories(services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
