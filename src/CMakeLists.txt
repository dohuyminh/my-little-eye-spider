find_package(llhttp REQUIRED)
find_package(stduuid REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(re2 REQUIRED)
find_package(libidn2 REQUIRED)

add_subdirectory(memory)
add_subdirectory(crawler)
add_subdirectory(services)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main 
    memory
    crawler
    services
    llhttp::llhttp
    stduuid::stduuid
)