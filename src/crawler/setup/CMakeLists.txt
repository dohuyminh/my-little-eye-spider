add_library(
  setup 
  STATIC
    frontier/default_frontier_return_type.cpp
    frontier/default_front_prioritizer.cpp 
    frontier/default_front_selector.cpp
    frontier/default_back_router.cpp
    frontier/default_back_selector.cpp
    frontier/politeness_tracker.cpp 
    worker/default_worker.cpp 
    worker/rewards.cpp 
    setup.cpp
)

target_link_libraries(setup PUBLIC components types)

target_include_directories(setup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
