add_executable(csv_to_db csv_to_db.cpp)
target_include_directories(csv_to_db PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(csv_to_db
    spdlog::spdlog
    absl::flags
    absl::flags_parse
)

add_executable(db_to_csv db_to_csv.cpp)
target_include_directories(db_to_csv PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(db_to_csv
    spdlog::spdlog
    absl::flags
    absl::flags_parse
)

set_target_properties(csv_to_db PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(db_to_csv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)