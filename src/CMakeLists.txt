add_library(engine schema.cpp csv_reader.cpp csv_writer.cpp type.cpp columnar_writer.cpp columnar_reader.cpp metadata.cpp column.cpp)
target_link_libraries(engine
    spdlog::spdlog
)

add_executable(csv_to_columnar csv_to_columnar.cpp)
target_include_directories(csv_to_columnar PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(csv_to_columnar
    engine
    spdlog::spdlog
    absl::flags
    absl::flags_parse
)

add_executable(columnar_to_csv columnar_to_csv.cpp)
target_include_directories(columnar_to_csv PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(columnar_to_csv
    engine
    spdlog::spdlog
    absl::flags
    absl::flags_parse
)

set_target_properties(csv_to_columnar PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(columnar_to_csv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)