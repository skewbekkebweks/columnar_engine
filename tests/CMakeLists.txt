add_executable(test test_csv_reader.cpp test_csv_writer.cpp test_schema.cpp)
target_include_directories(test PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test
    engine
    spdlog::spdlog
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)