add_library(operators Block.cpp Scan.cpp Count.cpp)
target_link_libraries(operators engine)
target_include_directories(operators PUBLIC ${CMAKE_SOURCE_DIR}/src)
