cmake_minimum_required(VERSION 2.6)
project(BackTracking)
enable_testing()
find_package (Threads)

ADD_DEFINITIONS ( -Wall -DDEBUG -pedantic -Wextra -std=c++11 -g -D_GLIBCXX_DEBUG )

ADD_DEFINITIONS ( -DTEST_ONLY_LIST )
add_executable(BasicTest versioned_graph.h versioned_graph_test.h versioned_graph_impl.h versioned_graph_non_members.h basic_tests.cpp)
add_executable(VersionedAdjacencyMatrixTest versioned_graph.h versioned_graph_test.h versioned_graph_impl.h  versioned_adjacency_matrix_test.cpp)
add_executable(VersionedAdjacencyListTest versioned_graph.h versioned_graph_non_members.h versioned_adjacency_list_test.cpp)
add_executable(Example example.cpp)
add_executable(Example1 example1.cpp)


target_link_libraries(BasicTest gtest gtest_main pthread)
target_link_libraries(VersionedAdjacencyMatrixTest gtest gtest_main pthread)
target_link_libraries(VersionedAdjacencyListTest gtest gtest_main pthread)

add_test(NAME BasicTest COMMAND BasicTest)
add_test(NAME VersionedAdjacencyMatrix COMMAND VersionedAdjacencyMatrixTest)
add_test(NAME VersionedAdjacencyList COMMAND VersionedAdjacencyListTest)
