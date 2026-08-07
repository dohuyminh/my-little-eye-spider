add_library(database
    postgresql_adapter.cpp
    postgresql_work.cpp
    postgresql_work_queue.cpp 
    
    models/url.cpp
    models/url_contents.cpp 
    models/url_metadata.cpp 
    models/url_outlinks.cpp
    models/robots_txt.cpp
)

target_link_libraries(
    database
    PUBLIC
      concurrentqueue::concurrentqueue
      libpqxx::pqxx
      Boost::headers
      cpp_dotenv
)

target_include_directories(database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
