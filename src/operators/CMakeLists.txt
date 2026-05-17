add_library(operators block.cpp scan.cpp count.cpp expression.cpp filter.cpp aggregator.cpp aggregate.cpp)
target_link_libraries(operators engine)
target_include_directories(operators PUBLIC ${CMAKE_SOURCE_DIR}/src)
