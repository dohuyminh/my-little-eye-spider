find_package(llhttp REQUIRED)
find_package(stduuid REQUIRED)
find_package(concurrentqueue REQUIRED)

add_subdirectory(memory)
add_subdirectory(crawler)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main 
    memory
    crawler
    llhttp::llhttp
    stduuid::stduuid
)