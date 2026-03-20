# Enable testing
enable_testing()

# Find GTest
find_package(GTest REQUIRED)

# Collect test files based on SUITE variable
if(SUITE STREQUAL "")
    # Run all tests
    file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp")
else()
    # Run specific test suite
    file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SUITE}/*.cpp")
endif()

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(
    run_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        memory
        types
        services
        components
        crawler
)

# Compile options
target_compile_options(
    run_tests
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -g
)

# Include directories
target_include_directories(
    run_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Register test with CTest
add_test(
    NAME run_tests
    COMMAND run_tests
)
