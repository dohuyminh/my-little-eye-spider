find_package(llhttp REQUIRED)
find_package(stduuid REQUIRED)

add_subdirectory(memory)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main 
    memory
    llhttp::llhttp
    stduuid::stduuid
)