#include "gtest/gtest.h"
#include "subgraph_test.h"
#include "subgraph.h"
using namespace boost;
using namespace std;

TEST(SubgraphTest, checkIfDetectIsomorphismInBidirectional) {
    typedef adjacency_list<vecS, vecS, bidirectionalS,external_data,external_data> graph_type;

    // Build graph1
    graph_type graph1;
    init_triangle(graph1);

    // Build graph2
    graph_type graph2;
    init_diamond(graph2);

    // Create callback to print mappings
    vf2_print_callback<graph_type, graph_type> callback(graph1, graph2);

    // Print out all subgraph isomorphism mappings between graph1 and graph2.
    // Vertices and edges are assumed to be always equivalent.
    ASSERT_TRUE(vf2_subgraph_iso(graph1, graph2, callback));

    counter_graph_callback<graph_type, graph_type> callback2(graph1, graph2);
    ASSERT_TRUE(vf2_subgraph_iso(graph1, graph2, callback2));
    ASSERT_EQ(2,callback2.get_count());
    cout << "value reset" <<  endl;
    counter_graph_callback<graph_type, graph_type>::count = 0;
    auto equivalent = make_attribute_equals_equivalent(graph1,graph2);
    ASSERT_EQ(0,callback2.get_count());
    ASSERT_TRUE(vf2_subgraph_iso(graph1, graph2, callback2,equivalent,equivalent));
    ASSERT_EQ(1,callback2.get_count());
}
TEST(SubgraphTest, checkIfDetectIsomorphismInBidirectionalWithEdges) {
    typedef adjacency_list<vecS, vecS, bidirectionalS,external_data,external_data> graph_type;

    // Build graph1
    graph_type graph1;
    init_triangle(graph1);

    // Build graph2
    graph_type graph2;
    init_diamond(graph2);

    graph1[edge(0,2,graph1).first].value = 1;
    graph2[edge(0,2,graph2).first].value = 1;

    counter_graph_callback<graph_type, graph_type> callback(graph1, graph2);
    counter_graph_callback<graph_type, graph_type>::count = 0;
    ASSERT_EQ(0,callback.get_count());
    auto equivalent = make_attribute_equals_equivalent(graph1,graph2);
    ASSERT_TRUE(vf2_subgraph_iso(graph1, graph2, callback,equivalent,equivalent));
    ASSERT_EQ(1,callback.get_count());
    counter_graph_callback<graph_type, graph_type>::count = 0;
    ASSERT_EQ(0,callback.get_count());
    graph2[edge(0,2,graph2).first].value = 0;
    graph2[edge(0,1,graph2).first].value = 0;
    ASSERT_TRUE(vf2_subgraph_iso(graph1, graph2, callback,equivalent,equivalent));
    ASSERT_EQ(0,callback.get_count());
}


TEST(SubgraphTest, checkIfDetectIsomorphismInUndirected) {
    typedef adjacency_list<vecS, vecS, undirectedS,external_data,external_data> graph_type2;

    // Build graph1
    graph_type2 graph3;
    init_triangle(graph3);

    // Build graph2
    graph_type2 graph4;
    init_diamond(graph4);
    counter_graph_callback<graph_type2, graph_type2> callback3(graph3, graph4);
    ASSERT_EQ(0,callback3.get_count());
    auto equivalent2 = make_attribute_equals_equivalent(graph3,graph4);
    ASSERT_TRUE(vf2_subgraph_iso(graph3, graph4, callback3,equivalent2,equivalent2));
    ASSERT_EQ(2,callback3.get_count());
}
TEST(SubgraphTest, checkIfDetectIsomorphismInUndirectedWithEdges) {
    typedef adjacency_list<vecS, vecS, undirectedS,external_data,external_data> graph_type2;

    // Build graph1
    graph_type2 graph3;
    init_triangle(graph3);

    // Build graph2
    graph_type2 graph4;
    init_diamond(graph4);

    graph3[edge(0,2,graph3).first].value = 1;
    graph4[edge(0,2,graph4).first].value = 1;

    counter_graph_callback<graph_type2, graph_type2> callback(graph3, graph4);
    counter_graph_callback<graph_type2, graph_type2>::count = 0;
    ASSERT_EQ(0,callback.get_count());
    auto equivalent2 = make_attribute_equals_equivalent(graph3,graph4);
    ASSERT_TRUE(vf2_subgraph_iso(graph3, graph4, callback,equivalent2,equivalent2));
    ASSERT_EQ(1,callback.get_count());

}

