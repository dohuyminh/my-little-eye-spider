find_package(llhttp REQUIRED)
find_package(stduuid REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(re2 REQUIRED)
find_package(libidn2 REQUIRED)
find_package(libpsl REQUIRED)

add_subdirectory(memory)
add_subdirectory(crawler)
add_subdirectory(services)

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
        -O3
)

target_link_libraries(
    main 
    memory
    crawler
    services
    types
    llhttp::llhttp
    stduuid::stduuid
)