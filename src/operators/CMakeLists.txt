add_library(operators block.cpp scan.cpp count.cpp expression.cpp filter.cpp aggregator.cpp aggregate.cpp group_by.cpp order_by.cpp limit.cpp project.cpp)
target_link_libraries(operators engine unordered_dense)
target_include_directories(operators PUBLIC ${CMAKE_SOURCE_DIR}/src)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_source_files_properties(aggregator.cpp PROPERTIES COMPILE_OPTIONS -Wno-array-bounds)
endif()
