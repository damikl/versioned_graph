//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
//#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/topological_sort.hpp>
#include "versioned_graph.h"
//#include "subgraph_test.h"
//#include "isomorphism_checking.h"
#include "gtest/gtest.h"

using namespace std;


TEST(VersionedGraphTest, conceptCheck){
    FILE* log_fd = fopen( "inner_check_concepts.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;

    typedef versioned_graph<boost::vecS, boost::vecS, boost::undirectedS,int,int> simple_graph;
    simple_graph sg;
    typedef typename graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<simple_graph>::edge_descriptor edge_descriptor;
    typedef typename graph_traits<simple_graph>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<simple_graph>::edge_iterator edge_iterator;
    typedef typename graph_traits<simple_graph>::out_edge_iterator out_edge_iterator;
    typedef typename graph_traits<simple_graph>::adjacency_iterator adjacency_iterator;
    typedef typename simple_graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<simple_graph>::directed_category directed_category;
    bool directed_category_res = std::is_same<boost::undirected_tag,directed_category>::value;
    ASSERT_TRUE(directed_category_res);
    typedef typename graph_traits<simple_graph>::edge_parallel_category edge_parallel_category;
    bool edge_parallel_category_res = std::is_same<boost::allow_parallel_edge_tag,edge_parallel_category>::value;
    ASSERT_TRUE(edge_parallel_category_res);
    typedef typename graph_traits<simple_graph>::vertices_size_type vertices_size_type;
    typedef typename graph_traits<simple_graph>::edges_size_type edges_size_type;
    typedef typename graph_traits<simple_graph>::degree_size_type degree_size_type;
    typedef typename simple_graph::out_edge_list_selector out_edge_list_selector;
    typedef typename simple_graph::vertex_list_selector vertex_list_selector;
    typedef typename simple_graph::directed_selector directed_selector;
    typedef typename simple_graph::edge_list_selector edge_list_selector;


    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(4,sg);
    edge_descriptor e;
    bool result = false;
    boost::tie(e,result) = add_edge(v1,v2,9,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v1,v3,8,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v2,v4,7,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v1,v4,11,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v2,v3,12,sg);
    assert(result);
    commit(sg);
    boost::remove_edge(e,sg);
    {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(sg);
        ASSERT_EQ(4,std::distance(vi.first,vi.second));
        ASSERT_EQ(4,boost::num_vertices(sg));
    }
    {
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(sg);
        ASSERT_EQ(4,boost::num_edges(sg));
        ASSERT_EQ(4,std::distance(ei.first,ei.second));

        // count edges marked as removed
        ASSERT_EQ(5,boost::num_edges(sg.get_self()));
        auto priv_ei = edges(sg.get_self());
        ASSERT_EQ(5,std::distance(priv_ei.first,priv_ei.second));
    }
    {
        std::pair<out_edge_iterator, out_edge_iterator> ei = out_edges(v1,sg);
        unsigned int count = 0;
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v1,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(3,count);
        count = 0;
        ei = out_edges(v2,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v2,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(2,count);
        count = 0;
        ei = out_edges(v3,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v3,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(1,count);
        count = 0;
//        ASSERT_TRUE(ei.first==ei.second);
        ei = out_edges(v4,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v4,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(2,count);
//        ASSERT_TRUE(ei.first==ei.second);
    }
    {
        std::pair<adjacency_iterator, adjacency_iterator> ei = adjacent_vertices(v1,sg);
        unsigned int count = 0;
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v1 << ": " << v;
            ASSERT_TRUE(v==v2 || v==v3 || v==v4);
        }
        ASSERT_EQ(3,count);
        count = 0;
        ei = adjacent_vertices(v2,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v2 << ": " << v;
            ASSERT_TRUE(v==v4 || v==v1);
        }
        ASSERT_EQ(2,count);
        count = 0;
        ei = adjacent_vertices(v3,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v3 << ": " << v;
            ASSERT_TRUE(v==v1);
        }
        ASSERT_EQ(1,count);
        count = 0;
        ei = adjacent_vertices(v4,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v4 << ": " << v;
            ASSERT_TRUE(v==v1 || v==v2);
        }
        ASSERT_EQ(2,count);
    }
    {
        std::pair<inv_adjacency_iterator, inv_adjacency_iterator> ei = inv_adjacent_vertices(v1,sg);
        unsigned int count = 0;
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v1 << ": " << v;
            ASSERT_TRUE(v==v2 || v==v3 || v==v4);
        }
        ASSERT_EQ(3,count);
        count = 0;
        ei = inv_adjacent_vertices(v2,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v2 << ": " << v;
            ASSERT_TRUE(v==v4 || v==v1);
        }
        ASSERT_EQ(2,count);
        count = 0;
        ei = inv_adjacent_vertices(v3,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v3 << ": " << v;
            ASSERT_TRUE(v==v1);
        }
        ASSERT_EQ(1,count);
        count = 0;
        ei = inv_adjacent_vertices(v4,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v4 << ": " << v;
            ASSERT_TRUE(v==v1 || v==v2);
        }
        ASSERT_EQ(2,count);
    }
}

TEST(VersionedGraphTest, SimpleExample) {
    FILE* log_fd = fopen( "inner_basic_tests.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,int,int> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(4,sg);
    add_edge(v1,v2,9,sg);
    add_edge(v1,v3,8,sg);
    add_edge(v2,v4,7,sg);
    add_edge(v1,v4,11,sg);
    add_edge(v2,v3,12,sg);
    sg[v4] = 4;
    ASSERT_EQ(4,num_vertices(sg));
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    sg[v4] = 5;
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    ASSERT_EQ(4,sg[v4]);
    FILE_LOG(logDEBUG1) << "attribute match";
    vertex_descriptor v5 = add_vertex(5,sg);
    add_edge(v5,v4,13,sg);
    add_edge(v3,v5,14,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
    ASSERT_EQ(5,sg[v5]);
}

TEST(VersionedGraphTest, withoutTypes) {
    FILE* log_fd = fopen( "checkWithoutTypes.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
}

TEST(VersionedGraphTest, normalTopologicalSort) {
    FILE* log_fd = fopen( "normalTopologicalSort.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef property<vertex_color_t, int> ColorProperty;
    typedef adjacency_list<boost::vecS, boost::vecS, boost::directedS,ColorProperty> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
//    commit(sg);
//    FILE_LOG(logDEBUG1) << "commit end";
//    remove_edge(v1,v4,sg);
//    ASSERT_EQ(4,num_edges(sg));
//    EXPECT_FALSE(edge(v1,v4,sg).second);
//    undo_commit(sg);
//    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));


    FILE_LOG(logDEBUG1) << "make tolological sort";
    // Perform a topological sort.
    std::vector<vertex_descriptor> topo_order;
    boost::topological_sort(sg, std::back_inserter(topo_order));
    // Print the results.
    FILE_LOG(logDEBUG1) << "Print the results";
    for(std::vector<vertex_descriptor>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        FILE_LOG(logDEBUG1) << *i;
    }

}

TEST(VersionedGraphTest, checkTopologicalSort) {
    FILE* log_fd = fopen( "checkTopologicalSort.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef property<vertex_color_t, int> ColorProperty;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,ColorProperty> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));


    FILE_LOG(logDEBUG1) << "make tolological sort";
    // Perform a topological sort.
    std::deque<int> topo_order;
    boost::topological_sort(sg.get_self(), std::front_inserter(topo_order));
    // Print the results.
    FILE_LOG(logDEBUG1) << "Print the results";
    for(std::deque<int>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        FILE_LOG(logDEBUG1) << *i;
    }

}
