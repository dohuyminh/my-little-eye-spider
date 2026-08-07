find_package(Boost REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(re2 REQUIRED)
find_package(libidn2 REQUIRED)
find_package(libpsl REQUIRED)
find_package(CURL REQUIRED)
find_package(lexbor REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libpqxx REQUIRED)

add_subdirectory(memory)
add_subdirectory(crawler)
add_subdirectory(dotenv)

add_executable(
    main
    main.cpp
)

target_compile_options(
    main
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-interference-size
        -O3
        -g # REMOVE LATER
)

target_link_libraries(
    main 
    memory
    crawler
    services
    setup
    types
    
    # dotenv
    cpp_dotenv
)
