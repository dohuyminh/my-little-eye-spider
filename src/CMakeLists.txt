find_package(llhttp REQUIRED)
find_package(stduuid REQUIRED)

add_subdirectory(networking)
add_subdirectory(memory)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main 
    networking
    memory
    llhttp::llhttp
    stduuid::stduuid
)