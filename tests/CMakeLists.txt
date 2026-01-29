add_executable(test test.cpp)
target_include_directories(test PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test
    add_lib 
    spdlog::spdlog
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test)

set_target_properties(tes PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)