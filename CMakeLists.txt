cmake_minimum_required(VERSION 3.5)
project(little-eye-spider)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MODE and SUITE cache variables
set(MODE "app" CACHE STRING "Build mode: 'app' for main executable, 'test' for unit tests")
set(SUITE "" CACHE STRING "Test suite path relative to tests/ (e.g., 'services' for tests/services/*)")

# Enable testing
enable_testing()

# Always add src to build libraries
add_subdirectory(src)

if(MODE STREQUAL "Test")
    # Build tests
    add_subdirectory(tests)
elseif(MODE STREQUAL "App")
    # Build main application (already added via src)
elseif()
    message(FATAL_ERROR "Unknown MODE: ${MODE}. Use 'app' or 'test'.")
endif()
