add_library(setup STATIC setup.cpp)

target_link_libraries(setup PUBLIC components types)

target_include_directories(setup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
