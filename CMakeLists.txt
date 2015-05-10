cmake_minimum_required(VERSION 2.6)
project(BackTracking)
enable_testing()
find_package (Threads)

ADD_DEFINITIONS ( -Wall -DDEBUG -pedantic -Wextra -std=c++1y -g -ftemplate-backtrace-limit=0 )

ADD_DEFINITIONS ( -DTEST_ONLY_LIST )
add_executable(VerticesTestList vertices_list_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h vertices_test.h )
add_executable(EdgesTestList edges_list_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h edges_test.h mapping.h utils.h)
add_executable(SimpleTestList simple_list_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h simple_test.h simple_key.h unordered_history_holder.h)

target_link_libraries(VerticesTestList gtest gtest_main pthread)
target_link_libraries(EdgesTestList gtest gtest_main pthread)
target_link_libraries(SimpleTestList gtest gtest_main pthread)

ADD_DEFINITIONS ( -DTEST_ONLY_VEC )
add_executable(VerticesTestVec vertices_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h key.h)
add_executable(EdgesTestVec edges_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h)
add_executable(SimpleTestVec simple_test.cpp archive.h  handler.h  history_holder.h  log.h  tests.h)

target_link_libraries(VerticesTestVec gtest gtest_main pthread)
target_link_libraries(EdgesTestVec gtest gtest_main pthread)
target_link_libraries(SimpleTestVec gtest gtest_main pthread)

add_test(NAME Simple COMMAND SimpleTestVec)
add_test(NAME SimpleList COMMAND SimpleTestList)
add_test(NAME Vertices COMMAND VerticesTestVec)
add_test(NAME VerticesList COMMAND VerticesTestList)
add_test(NAME Edges COMMAND EdgesTestVec)
add_test(NAME EdgesList COMMAND EdgesTestList)
