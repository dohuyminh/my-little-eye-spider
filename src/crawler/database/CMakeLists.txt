add_library(db_adapters
    postgresql_adapter.cpp
)

target_link_libraries(
    db_adapters
    libpqxx::pqxx
)

target_include_directories(db_adapters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)